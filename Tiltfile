allow_k8s_contexts('default')

# Redis ConfigMap and other k8s resources
k8s_yaml('kube/starcry-dev.yaml')

# Compile the binary
local_resource(
    'starcry-binary',
    cmd='make build',
    deps=['src/'],
    labels=['build']
)

# Define the main starcry image - no need to rebuild it
docker_build(
    'docker.io/rayburgemeestre/starcry',
    context='.',
    live_update=[
        # sync('./starcry', '/binary/starcry'),  # Sync the binary directly to the mounted path
        sync('./build', '/workdir/build'),
        sync('./input', '/workdir/input'),
        # sync('./web', '/workdir/web'),
    ]
)

local_resource(
    'starcry-binary-watcher',
    cmd='touch ./build/starcry.stop; sleep 5; rm -rfv ./build/starcry.stop',
    deps=['build/starcry'],
    labels=['build']
)

# Resource definitions
k8s_resource(
    'starcry',
    port_forwards=['18080:18080', '10000:10000'],
    resource_deps=['redis', 'starcry-binary'],
    labels=['app']
)

k8s_resource(
    'redis',
    port_forwards=['6379:6379'],
    labels=['database']
)

k8s_resource(
    'starcry-workers',
    resource_deps=['starcry', 'redis'],
    labels=['workers']
)

# Quasar development server
docker_build(
    'docker.io/rayburgemeestre/quasar-dev',
    context='./web',
    dockerfile_contents='''
    FROM node:18-alpine
    WORKDIR /app
    COPY package*.json ./
    RUN npm install
    COPY . .
    RUN npm install -g @quasar/cli
    EXPOSE 9000
    CMD ["quasar", "dev"]
    ''',
    live_update=[
        sync('./web/src', '/app/src'),
        sync('./web/public', '/app/public'),
    ]
)

k8s_resource(
    'starcry-web',
    port_forwards=['9000:9000'],
    labels=['web']
)