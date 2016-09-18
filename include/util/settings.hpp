/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>

#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

class settings
{
public:

    struct user_settings {
        uint16_t gui_port = 0;

        template <class Archive>
        void serialize( Archive & ar )
        {
            ar( gui_port );
        }
    } user;

    void load() {
        try {
            struct passwd *pw = getpwuid(getuid());
            const char *homedir = pw->pw_dir;
            std::ifstream is(std::string(homedir) + "/.starcry.conf");
            if (is.good()) {
                cereal::BinaryInputArchive read_settings(is);
                read_settings(user);
                is.close();
            }
        }
        catch (cereal::Exception &ex) {
            std::cout << "Failure loading configuration from ~/.starcry.conf: " << ex.what() << endl;
        }
    }

    void save() {
        try {
            struct passwd *pw = getpwuid(getuid());
            const char *homedir = pw->pw_dir;
            std::ofstream os(std::string(homedir) + "/.starcry.conf");
            cereal::BinaryOutputArchive write_settings(os);
            write_settings(user);
            os.close();
            std::cout << "wrote configuration file to ~/.starcry.conf" << endl;
        }
        catch (cereal::Exception &ex) {
            std::cout << "Failure saving configuration to ~/.starcry.conf: " << ex.what() << endl;
        }
        catch (std::exception &ex) {
            std::cout << "Unexpected failure saving configuration to ~/.starcry.conf: " << ex.what() << endl;
        }
        catch (...) {
            std::cout << "Unknown failure saving configuration file to ~/.starcry.conf" << endl;
        }
    }

};

