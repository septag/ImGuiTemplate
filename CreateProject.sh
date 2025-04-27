#!/bin/bash

pushd "$(dirname "$0")" > /dev/null

if [ -z "$1" ]; then
    echo "Error: Project name is required."
    exit 1
fi

if [ -d "../$1" ]; then
    echo "Error: Target directory ../$1 already exists"
    popd > /dev/null
    exit 1
fi

mkdir -p "../$1" 2>/dev/null
rsync -a --exclude=.git --exclude=.build --exclude=build --exclude=bin --exclude=CreateProject.bat . "../$1/"

popd > /dev/null

if [ $? -ge 8 ]; then
    echo "Error: rsync failed with exit code $?"
    exit $?
fi

cd "$(dirname "$0")/../$1"

# Configure
mkdir -p .build
pushd .build > /dev/null
cmake ../cmake -GNinja -DAPPNAME="$1"
popd > /dev/null

# Create configure.sh file
cat > Configure.sh <<EOF
#!/bin/bash
if [ ! -d ".build" ]; then
    mkdir -p .build
fi
pushd .build > /dev/null
cmake ../cmake -GNinja -DAPPNAME=$1 "\$@"
popd > /dev/null
EOF

chmod +x Configure.sh

echo "Modifying .vscode/settings.json"
sed -i "s/-DAPPNAME=Test/-DAPPNAME=$1/g" .vscode/settings.json

echo "Copy of template project created: $1"
echo "Run/Edit Configure.sh for reconfiguring and adding extra options"