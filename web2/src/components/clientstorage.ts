function uuidv4() {
  return ([1e7] + -1e3 + -4e3 + -8e3 + -1e11).replace(/[018]/g, (c) =>
    (
      c ^
      (crypto.getRandomValues(new Uint8Array(1))[0] & (15 >> (c / 4)))
    ).toString(16)
  );
}

export function load_client_data() {
  return localStorage.getItem('client-data')
    ? JSON.parse(localStorage.getItem('client-data'))
    : {
        ID: uuidv4(),
      };
}

export function save_client_data(client_data) {
  localStorage.setItem('client-data', JSON.stringify(client_data));
}
