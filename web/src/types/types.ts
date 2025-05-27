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
