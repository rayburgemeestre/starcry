#!/bin/bash

set -ex

docker login

current_version=$(cat .version)
next_version=$((current_version + 1))

echo current: $current_version
echo next: $next_version

echo $next_version > .version

echo going to compile...
make publish

echo going to push...
docker push rayburgemeestre/starcry:v`cat .version`

echo going to update deployment...

sed -i.bak "s/:v$current_version/:v$next_version/g" README.md
sed -i.bak "s/:v$current_version/:v$next_version/g" kube/starcry.yaml
timeout 5s kubectl apply -f kube/starcry.yaml || true

echo going to commit...

git add .version
git add kube/starcry.yaml
git add README.md

git commit -m "Bump docker image to version v$next_version"

git tag v$next_version

git push origin v$next_version
git push origin2 v$next_version
