import { StarcryAPI } from 'src/core/api';
import { useObjectsStore } from 'stores/objects';

export function create_object_endpoint() {
  const objects_store = useObjectsStore();

  const self = new StarcryAPI(
    'objects',
    StarcryAPI.json_type,
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    (msg) => {},
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    (json) => {
      // temporary switched to binary, as the plan was to deserialize binary in wasm
      // const decoder = new TextDecoder();
      // const json = JSON.parse(decoder.decode(buffer));
      console.log(json);
      objects_store.objects = json['shapes'];
      objects_store.updateLookupTable();
    },
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    (_) => {},
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    (_) => {}
  );
  return self;
}
