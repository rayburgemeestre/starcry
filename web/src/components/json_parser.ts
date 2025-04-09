export class JsonWithObjectsParser {
  private json_str: string;
  private obj: undefined;
  private remainder: string;
  private functions: any[];
  private constants: string;

  constructor(json_str: string, constants: string) {
    this.json_str = json_str;
    this.json_str = this.json_str.substring(this.json_str.indexOf('{'));
    this.obj = undefined;
    this.functions = [];
    this.remainder = '';
    this.constants = constants;
    this._parse();
  }
  parsed() {
    if (!this.obj) {
      this.obj = {};
    }
    if (!this.obj.video) {
      this.obj['video'] = {};
    }
    if (!this.obj.preview) {
      this.obj['preview'] = {};
    }
    if (!this.obj.scenes) {
      this.obj['scenes'] = [];
    }
    if (!this.obj.gradients) {
      this.obj['gradients'] = {};
    }
    if (!this.obj.textures) {
      this.obj['textures'] = {};
    }
    return this.obj;
  }

  update_function(index: number, value: string) {
    for (let i = 0; i < this.functions.length; i++) {
      if ('FUNCTION ' + i !== index.toString()) {
        continue;
      }
      this.functions[i] = value;
    }
  }

  to_string() {
    const obj = this.parsed();
    let str = JSON.stringify(obj, null, 2);
    let num = 0;
    for (const fun of this.functions) {
      str = str.replace('"FUNCTION ' + num + '"', fun);
      num++;
    }

    // in str replace all strings with a regex like this:
    // input: "@@foo.bar@@" should be replaced with foo.bar
    str = str.replace(/"@@(.*?)@@"/g, '$1');

    return 'script = ' + str + '\n;\n' + this.remainder;
  }

  fun(lookup: string) {
    const index = parseInt(lookup.split(' ')[1]);
    if (this.functions.length < index) {
      return 'OUT OF BOUNDS';
    }
    return this.functions[index];
  }
  _parse() {
    // let's get this JSON in parseable shape
    this.json_str = this.json_str.replaceAll("'", '"');

    let trail = '';
    let in_cpp_comment = false;
    let in_c_comment = false;
    let in_string: string | boolean = false;
    let in_function: number | boolean = false;
    let brace = 0;
    let bracket = 0;
    let function_num = 0;
    const functions: string[] = [];
    let function_str = '';
    let previous_char = ' ';
    let previous_char_pos = 0;
    let previous_char2 = ' ';
    type Flags = {
      blending_type: boolean;
      zernike_type: boolean;
      texture_effect: boolean;
    };

    type FlagsDictionary = {
      [key: string]: boolean;
    };

    const flags: FlagsDictionary = { blending_type: false, zernike_type: false, texture_effect: false };

    let stop = false;
    let remainder = '';
    for (let i = 0; i < this.json_str.length && !stop; i++) {
      const s: string = this.json_str[i];

      if (s === '*' && this.json_str.substring(i, i + '*/'.length) === '*/') {
        in_c_comment = false;
      }
      if (s === '/' && this.json_str.substring(i, i + '/*'.length) === '/*') {
        in_c_comment = true;
      }

      if (in_cpp_comment && s === '\n') {
        in_cpp_comment = false;
      }
      if (s === '/' && this.json_str.substring(i, i + '//'.length) === '//') {
        in_cpp_comment = true;
      }

      // skip spaces and tabs, unless inside a comment
      if (!(in_string as boolean) && !in_function) {
        if (s === ' ' || s === '\t') continue;
      }
      if (in_function !== false) {
        function_str += s;
      }
      // no support for escaping
      if (!in_c_comment && !in_cpp_comment && (s === '"' || s === "'")) {
        if (in_string === s) {
          in_string = false;
        } else {
          in_string = s;
        }
      }
      if (!(in_function as boolean)) {
        if (s === '[') {
          bracket++;
        }
        if (s === ']') {
          // eslint-disable-next-line @typescript-eslint/no-unused-vars
          bracket--;
          if (previous_char === ',') {
            // get rid of the ',', unfortunately quite tedious with javascript.
            if (trail[previous_char_pos - 1] === ',') {
              // if (false)
              //   trail =
              //     trail.substr(0, previous_char_pos - 1) +
              //     trail.substr(previous_char_pos);
            }
          }
        }
        if (s === '}') {
          if (previous_char === ',') {
            if (trail[previous_char_pos - 1] === ',') {
              // if (false)
              //   trail =
              //     trail.substr(0, previous_char_pos - 1) +
              //     trail.substr(previous_char_pos);
            }
          }
        }
      }
      if (s === '{') {
        brace++;
      }
      if (s === '}') {
        brace--;
        if (brace === in_function) {
          in_function = false;
          functions.push(function_str);
          continue;
        }
        if (brace == 0) {
          stop = true;
          remainder = this.json_str.substring(i + 1);
        }
      }
      //
      if (!(in_function as boolean) && s === 'f' && this.json_str.substring(i, i + 'function'.length) === 'function') {
        in_function = brace;
        trail += '"FUNCTION ' + function_num + '"';
        function_str = 'f';
        function_num++;
      }
      if (!in_string) {
        const handleType = function (key: string, flag: string, first_char: string) {
          // End of type
          if (flags[flag] && ',;\r\n'.indexOf(s) !== -1) {
            trail += '"';
            flags[flag] = false;
          }
          // Start of type
          if (
            !in_function &&
            !flags[flag] &&
            s === first_char &&
            this.json_str.substring(i, i + key.length) === key &&
            previous_char !== '"'
          ) {
            flags[flag] = true;
            trail += '"' + first_char; // stringize it
            return true; // Indicating that we should "continue" in the calling code.
          }
          return false; // Indicating no need to "continue" in the calling code.
        }.bind(this);

        // we will do a different approach
        // if (handleType.call(this, 'blending_type', 'blending_type', 'b')) continue;
        // if (handleType.call(this, 'zernike_type', 'zernike_type', 'z')) continue;
        // if (handleType.call(this, 'texture_effect', 'texture_effect', 't')) continue;
      }
      if (in_function === false) {
        if (',;'.indexOf(s) !== -1 && previous_char === '.' && '0123456789'.indexOf(previous_char2) !== -1) {
          trail += '0';
        }

        trail += s;
        if (s !== '\r' && s !== '\n') {
          previous_char2 = previous_char;
          previous_char = s;
          previous_char_pos = trail.length;
        }
      }
    }

    this.functions = functions;
    //
    // let obj = eval('(function() { return ' + this.json_str + '; })()');
    try {
      // if remainder starts with ;, strip it off
      if (remainder[0] === ';') {
        remainder = remainder.substring(1);
      }
      this.remainder = remainder;
      const eval_str =
        'true; ' +
        this.constants +
        '; for (let prop in sc_constant_values) {\n' +
        '  if (sc_constant_values.hasOwnProperty(prop)) {\n' +
        '    window[prop] = sc_constant_values[prop];\n' +
        '  }\n' +
        '};\n' +
        remainder +
        '; for (let prop in sc_constant_values_reflect) {\n' +
        '  if (sc_constant_values_reflect.hasOwnProperty(prop)) {\n' +
        '    window[prop] = sc_constant_values_reflect[prop];\n' +
        '  }\n' +
        '}; (function() { return ' +
        trail +
        '; })()';
      console.log(eval_str);
      this.obj = eval(eval_str);
    } catch (e) {
      console.log(trail);
      console.log(e);
    }
    // console.log(JSON.parse(trail));
  }
}
