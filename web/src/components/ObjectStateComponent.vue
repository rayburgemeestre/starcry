<template>
  <div v-if="currentObject">
    <q-card flat bordered class="q-mb-md">
      <q-card-section>
        <div class="text-h6">{{ currentObjectId }}</div>
        <div class="text-caption">Object State Controls</div>
      </q-card-section>
    </q-card>

    <!-- Emit Effects Section -->
    <q-expansion-item
      v-if="hasEmitEffects"
      expand-separator
      icon="arrow_upward"
      label="Emit Effects"
      header-class="text-primary"
      default-opened
    >
      <q-card>
        <q-card-section>
          <div v-for="(emitConfig, effectName) in currentObject.emit" :key="`emit-${effectName}`" class="q-mb-md">
            <q-item>
              <q-item-section>
                <q-item-label header>{{ effectName }}</q-item-label>
                <q-item-label caption>
                  Type: {{ emitConfig.type }} | Max: {{ emitConfig.max }}
                  <span v-if="emitConfig.range_start !== undefined || emitConfig.range_end !== undefined">
                    | Range: [{{ emitConfig.range_start || 0 }}, {{ emitConfig.range_end || 1 }}]
                  </span>
                </q-item-label>

                <div class="q-mt-md">
                  <q-item-label>Intensity Override</q-item-label>
                  <q-slider
                    v-model="emitIntensities[effectName]"
                    :min="0"
                    :max="1"
                    :step="0.01"
                    label
                    label-always
                    color="primary"
                    track-size="10px"
                    thumb-size="20px"
                    @update:model-value="updateEmitIntensity(effectName, $event)"
                  />
                  <div class="text-caption text-grey-7">
                    Output: {{ calculateEmitOutput(effectName, emitIntensities[effectName]) }}
                  </div>
                </div>
              </q-item-section>
            </q-item>
            <q-separator v-if="!isLastEmit(effectName)" />
          </div>
        </q-card-section>
      </q-card>
    </q-expansion-item>

    <!-- Absorb Effects Section -->
    <q-expansion-item
      v-if="hasAbsorbEffects"
      expand-separator
      icon="arrow_downward"
      label="Absorb Effects"
      header-class="text-secondary"
      default-opened
      class="q-mt-md"
    >
      <q-card>
        <q-card-section>
          <div v-for="(absorbConfig, absorbName) in currentObject.absorb" :key="`absorb-${absorbName}`" class="q-mb-md">
            <q-item>
              <q-item-section>
                <q-item-label header>{{ absorbName }}</q-item-label>
                <q-item-label caption>
                  Input: {{ absorbConfig.input }}
                  <span v-if="absorbConfig.range_start !== undefined || absorbConfig.range_end !== undefined">
                    | Range: [{{ absorbConfig.range_start || 0 }}, {{ absorbConfig.range_end || 1 }}]
                  </span>
                </q-item-label>

                <div class="q-mt-md">
                  <q-item-label>Input Intensity</q-item-label>
                  <q-slider
                    v-model="absorbIntensities[absorbName]"
                    :min="0"
                    :max="1"
                    :step="0.01"
                    label
                    label-always
                    color="secondary"
                    track-size="10px"
                    thumb-size="20px"
                    @update:model-value="updateAbsorbIntensity(absorbName, $event)"
                  />
                  <div class="text-caption text-grey-7">
                    Remapped: {{ calculateAbsorbOutput(absorbName, absorbIntensities[absorbName]) }}
                  </div>
                </div>

                <q-btn
                  flat
                  dense
                  size="sm"
                  color="secondary"
                  label="Trigger Effect"
                  @click="triggerAbsorbEffect(absorbName)"
                  class="q-mt-sm"
                />
              </q-item-section>
            </q-item>
            <q-separator v-if="!isLastAbsorb(absorbName)" />
          </div>
        </q-card-section>
      </q-card>
    </q-expansion-item>

    <!-- Object Properties Section -->
    <q-expansion-item
      expand-separator
      icon="settings"
      label="Object Properties"
      header-class="text-accent"
      class="q-mt-md"
    >
      <q-card>
        <q-card-section>
          <div class="row q-col-gutter-md">
            <div v-for="(value, prop) in displayProperties" :key="`prop-${prop}`" class="col-12 col-sm-6">
              <q-item dense>
                <q-item-section>
                  <q-item-label caption>{{ prop }}</q-item-label>
                  <q-item-label>{{ formatPropertyValue(value) }}</q-item-label>
                </q-item-section>
              </q-item>
            </div>
          </div>
        </q-card-section>
      </q-card>
    </q-expansion-item>

    <!-- Debug Output -->
    <q-expansion-item
      expand-separator
      icon="bug_report"
      label="Debug Output"
      header-class="text-grey-7"
      class="q-mt-md"
    >
      <q-card>
        <q-card-section>
          <pre class="text-caption">{{ debugOutput }}</pre>
        </q-card-section>
      </q-card>
    </q-expansion-item>
  </div>

  <div v-else class="text-center q-pa-lg text-grey-6">
    No object selected
    {{ currentObject }} ??
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, computed, watch, reactive } from 'vue';
import { useObjectsStore } from 'stores/objects';

export default defineComponent({
  name: 'ObjectStateComponent',
  props: {
    selectedObjectId: {
      type: String,
      default: '',
    },
  },
  setup(props) {
    const objectsStore = useObjectsStore();

    // Reactive state for intensity values
    const emitIntensities = reactive<Record<string, number>>({});
    const absorbIntensities = reactive<Record<string, number>>({});
    const debugOutput = ref('');

    // Get current object from store
    const currentObject = computed(() => {
      console.log('XXX: selectedObjectId', props.selectedObjectId);
      console.log('XXX: objectsStore.object', objectsStore.object);
      if (!props.selectedObjectId || !objectsStore.object) return null;
      console.log('XXX OK');
      return objectsStore.object;
    });

    const currentObjectId = computed(() => props.selectedObjectId);

    // Check if object has emit/absorb effects
    const hasEmitEffects = computed(() => {
      return currentObject.value?.emit && Object.keys(currentObject.value.emit).length > 0;
    });

    const hasAbsorbEffects = computed(() => {
      return currentObject.value?.absorb && Object.keys(currentObject.value.absorb).length > 0;
    });

    // Get display properties (excluding functions and internal properties)
    const displayProperties = computed(() => {
      if (!currentObject.value) return {};

      const props: Record<string, any> = {};
      const exclude = ['emit', 'absorb', 'init', 'time', 'subobj', 'props'];

      for (const [key, value] of Object.entries(currentObject.value)) {
        if (!exclude.includes(key) && typeof value !== 'function') {
          props[key] = value;
        }
      }

      return props;
    });

    // Initialize intensities when object changes
    watch(
      currentObject,
      (newObj) => {
        if (newObj) {
          // Initialize emit intensities
          if (newObj.emit) {
            for (const effectName in newObj.emit) {
              if (!(effectName in emitIntensities)) {
                emitIntensities[effectName] = 0.5; // Default to middle
              }
            }
          }

          // Initialize absorb intensities
          if (newObj.absorb) {
            for (const absorbName in newObj.absorb) {
              if (!(absorbName in absorbIntensities)) {
                absorbIntensities[absorbName] = 0.5; // Default to middle
              }
            }
          }
        }
      },
      { immediate: true }
    );

    // Calculate emit output based on range
    const calculateEmitOutput = (effectName: string, intensity: number) => {
      const emit = currentObject.value?.emit?.[effectName];
      if (!emit) return 0;

      const rangeStart = emit.range_start ?? 0;
      const rangeEnd = emit.range_end ?? 1;
      const output = rangeStart + intensity * (rangeEnd - rangeStart);

      return output.toFixed(3);
    };

    // Calculate absorb output based on range
    const calculateAbsorbOutput = (absorbName: string, intensity: number) => {
      const absorb = currentObject.value?.absorb?.[absorbName];
      if (!absorb) return 0;

      const rangeStart = absorb.range_start ?? 0;
      const rangeEnd = absorb.range_end ?? 1;
      const output = rangeStart + intensity * (rangeEnd - rangeStart);

      return output.toFixed(3);
    };

    // Update emit intensity
    const updateEmitIntensity = (effectName: string, value: number) => {
      emitIntensities[effectName] = value;
      updateDebugOutput();
    };

    // Update absorb intensity
    const updateAbsorbIntensity = (absorbName: string, value: number) => {
      absorbIntensities[absorbName] = value;
      updateDebugOutput();
    };

    // Trigger absorb effect manually
    const triggerAbsorbEffect = (absorbName: string) => {
      const absorb = currentObject.value?.absorb?.[absorbName];
      if (!absorb || !absorb.effect) return;

      const intensity = absorbIntensities[absorbName];
      const remappedIntensity = Number(calculateAbsorbOutput(absorbName, intensity));

      // Log the effect trigger
      console.log(`Triggering absorb effect '${absorbName}' with intensity ${remappedIntensity}`);
      debugOutput.value += `\nTriggered '${absorbName}' with intensity ${remappedIntensity}`;

      // Note: In a real implementation, this would call the effect function
      // with the proper context and emitter reference
    };

    // Update debug output
    const updateDebugOutput = () => {
      let output = `Object: ${currentObjectId.value}\n\n`;

      if (hasEmitEffects.value) {
        output += 'Emit Effects:\n';
        for (const [name, intensity] of Object.entries(emitIntensities)) {
          const outputValue = calculateEmitOutput(name, intensity);
          output += `  ${name}: intensity=${intensity.toFixed(2)} → output=${outputValue}\n`;
        }
        output += '\n';
      }

      if (hasAbsorbEffects.value) {
        output += 'Absorb Effects:\n';
        for (const [name, intensity] of Object.entries(absorbIntensities)) {
          const outputValue = calculateAbsorbOutput(name, intensity);
          output += `  ${name}: input=${intensity.toFixed(2)} → remapped=${outputValue}\n`;
        }
      }

      debugOutput.value = output;
    };

    // Helper methods
    const isLastEmit = (effectName: string) => {
      if (!currentObject.value?.emit) return true;
      const keys = Object.keys(currentObject.value.emit);
      return keys[keys.length - 1] === effectName;
    };

    const isLastAbsorb = (absorbName: string) => {
      if (!currentObject.value?.absorb) return true;
      const keys = Object.keys(currentObject.value.absorb);
      return keys[keys.length - 1] === absorbName;
    };

    const formatPropertyValue = (value: any) => {
      if (typeof value === 'object') {
        return JSON.stringify(value, null, 2);
      }
      return String(value);
    };

    // Initial debug output
    updateDebugOutput();

    return {
      currentObject,
      currentObjectId,
      hasEmitEffects,
      hasAbsorbEffects,
      displayProperties,
      emitIntensities,
      absorbIntensities,
      debugOutput,
      calculateEmitOutput,
      calculateAbsorbOutput,
      updateEmitIntensity,
      updateAbsorbIntensity,
      triggerAbsorbEffect,
      isLastEmit,
      isLastAbsorb,
      formatPropertyValue,
    };
  },
});
</script>

<style scoped>
pre {
  white-space: pre-wrap;
  word-wrap: break-word;
  margin: 0;
}

.q-slider {
  margin: 10px 0;
}
</style>
