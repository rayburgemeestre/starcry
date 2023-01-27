export class JsonWithObjectsParser {
  constructor(json_str: string) {
    this.json_str = json_str;
    this.obj = undefined;
    this.functions = [];
    this._parse();
  }
  parsed() {
    return this.obj;
  }
  funs() {
    return this.functions;
  }
  _parse() {
    console.log('handle:');
    // let's get this JSON in parseable shape
    this.json_str = this.json_str.replaceAll("'", '"');

    let trail = '';
    let in_cpp_comment = false;
    let in_c_comment = false;
    let in_string = false;
    let in_function = false;
    let brace = 0;
    let bracket = 0;
    let function_num = 0;
    const functions = [];
    let function_str = '';
    let previous_char = ' ';
    let previous_char_pos = 0;
    let previous_char2 = ' ';
    let in_blending_type = false;
    for (let i = 0; i < this.json_str.length; i++) {
      const s = this.json_str[i];

      if (s === '*' && this.json_str.substr(i, '*/'.length) === '*/') {
        in_c_comment = false;
      }
      if (s === '/' && this.json_str.substr(i, '/*'.length) === '/*') {
        in_c_comment = true;
      }

      if (in_cpp_comment && s === '\n') {
        in_cpp_comment = false;
      }
      if (s === '/' && this.json_str.substr(i, '//'.length) === '//') {
        in_cpp_comment = true;
      }

      // skip spaces and tabs, unless inside a comment
      if (in_string === false && in_function === false) {
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
      if (in_function === false) {
        if (s === '[') {
          bracket++;
        }
        if (s === ']') {
          // eslint-disable-next-line @typescript-eslint/no-unused-vars
          bracket--;
          if (previous_char === ',') {
            // get rid of the ',', unfortunately quite tedious with javascript.
            if (trail[previous_char_pos - 1] === ',') {
              if (false)
                trail =
                  trail.substr(0, previous_char_pos - 1) +
                  trail.substr(previous_char_pos);
            }
          }
        }
        if (s === '}') {
          if (previous_char === ',') {
            if (trail[previous_char_pos - 1] === ',') {
              if (false)
                trail =
                  trail.substr(0, previous_char_pos - 1) +
                  trail.substr(previous_char_pos);
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
      }
      //
      if (
        in_function === false &&
        s === 'f' &&
        this.json_str.substr(i, 'function'.length) === 'function'
      ) {
        in_function = brace;
        trail += '"FUNCTION ' + function_num + '"';
        function_str = 'f';
        function_num++;
      }
      if (!in_string) {
        if (in_blending_type === true && ',;'.indexOf(s) !== -1) {
          trail += '"';
          in_blending_type = false;
        }
        if (
          in_blending_type === false &&
          s === 'b' &&
          this.json_str.substr(i, 'blending_type'.length) === 'blending_type' &&
          previous_char !== '"'
        ) {
          in_blending_type = true;
          trail += '"b'; // stringize it
          continue;
        }
      }
      if (in_function === false) {
        if (
          ',;'.indexOf(s) !== -1 &&
          previous_char === '.' &&
          '0123456789'.indexOf(previous_char2) !== -1
        ) {
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

    // console.log(this.json_str);
    // console.log(trail);
    // console.log(functions);
    this.functions = functions;
    const obj = eval('(function() { return ' + trail + '; })()');
    //
    // let obj = eval('(function() { return ' + this.json_str + '; })()');
    this.obj = obj;
    // console.log(JSON.parse(trail));
  }
}
