export function append_script_to_body(filename: string) {
  const s = document.createElement('script');
  s.setAttribute('src', filename);
  document.body.appendChild(s);
}
