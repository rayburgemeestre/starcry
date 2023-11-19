import { StarcryAPI } from 'components/api';
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
    (buffer) => {
      objects_store.objects = buffer;
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
