#!/bin/python3

import os
import subprocess
import base64
import shutil

from datetime import datetime

def main():
    path_of_the_directory= 'output/expected'
    os.makedirs("output/diff", exist_ok=True)
    os.makedirs("output/diff2", exist_ok=True)

    with open('output/report.html', 'w') as f:
        f.write("<html>")
        f.write("<body>")
        f.write("<table border=1>")
        f.write("<tbody>")
        for filename in os.listdir(path_of_the_directory):
            file = os.path.join(path_of_the_directory, filename)
            if os.path.isfile(file) and file.endswith('.png'):
                expected = file
                observed = file.replace('expected', 'observed')
                diff = file.replace('expected', 'diff')
                diff2 = file.replace('expected', 'diff2')
                cmd = f"compare {expected} {observed} -highlight-color blue {diff} || true"
                subprocess.check_output(cmd, shell=True)
                cmd = f"convert '(' {expected} -flatten -grayscale Rec709Luminance ')' '(' {observed} -flatten -grayscale Rec709Luminance ')' '(' -clone 0-1 -compose darken -composite ')' -channel RGB -combine {diff2} || true"
                subprocess.check_output(cmd, shell=True)

                def process(path, name):
                    cmd = f"convert -resize 420x {path} /tmp/{name}.png || true"
                    subprocess.check_output(cmd, shell=True)
                    with open(f"/tmp/{name}.png", 'rb') as fd:
                        b64 = base64.b64encode(fd.read())
                        return b64.decode("utf-8")
                    return ""

                observed = process(observed, "observed")
                expected = process(expected, "expected")
                diff = process(diff, "diff")
                diff2 = process(diff2, "diff2")

                f.write("<tr>")
                f.write(f'<td colspan=4>{file}</td>')
                f.write("</tr>")
                f.write("<tr>")
                f.write(f'<td><img src="data:image/png;base64,{observed}"></td>')
                f.write(f'<td><img src="data:image/png;base64,{expected}"></td>')
                f.write(f'<td><img src="data:image/png;base64,{diff}"></td>')
                f.write(f'<td><img src="data:image/png;base64,{diff2}"></td>')
                f.write("</tr>")

        f.write("</tbody>")
        f.write("</table>")
        f.write("</body>")
        f.write("</html>")

    today = datetime.today().strftime('%d-%m-%Y')
    shutil.copyfile('output/report.html', f'output/report-{today}.html');


if __name__ == '__main__':
    main()
