/** Any value that survives a JSON round-trip. */
export type JsonValue = string | number | boolean | null | JsonValue[] | { [key: string]: JsonValue };
export type JsonObject = { [key: string]: JsonValue };

// The /stats websocket multiplexes three message kinds. Shapes below mirror what the
// engine actually emits: metrics.cpp (metrics), webserver.cpp send_stats (stats) and
// starcry.cpp (fs_change).
export interface MetricsThread {
  name: string;
  state: string;
  /** only present while the thread is idle */
  seconds_idle?: number;
  /** only present while the thread is busy */
  job?: { job: number; chunk: number; seconds_busy: number };
}

export interface MetricsJob {
  number: number;
  state: string;
  gen_time?: number;
  render_time?: number;
}

export interface PiperNodeStats {
  name: string;
  counter: number;
}

/** The /viewpoint endpoint echoes back a serialized data::viewpoint. */
export interface ViewpointMessage {
  scale: number;
  raw: boolean;
  preview: boolean;
  save: boolean;
  labels: boolean;
  caching: boolean;
  debug: boolean;
  canvas_w: number;
  canvas_h: number;
  offset_x: number;
  offset_y: number;
}

export type StatsMessage =
  | { type: 'metrics'; data: { threads?: MetricsThread[]; jobs?: MetricsJob[] } }
  | { type: 'stats'; data: PiperNodeStats[] }
  | { type: 'fs_change'; file: string; source: string };

export type ObjectDef = {
  id: string;
  x: number;
  y: number;
  z: number;
  // props: Record<string, any>;
};

export type Scene = {
  name: string;
  duration: number;
  objects: ObjectDef[];
};

export interface GradientStop {
  position: number;
  r: number;
  g: number;
  b: number;
  a: number;
}

export interface GradientsMap {
  [key: string]: string | GradientStop[];
}

export interface StringFormatMap {
  [key: string]: boolean;
}

export interface ParsedScript {
  // gradients?: GradientsMap;
  [key: string]: any;
}
