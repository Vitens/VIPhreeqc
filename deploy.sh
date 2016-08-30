#!/bin/bash
echo "Running deployment script..."

CURRENT_COMMIT=`git rev-parse HEAD`

# Change the branch used if applicable (e.g. gh-pages)
echo "Cloning master branch..."

# Hide output since we use an access token here
git clone -b master "https://${GH_TOKEN}@${GH_REF}" _deploy > /dev/null 2>&1 || exit 1

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then export RELEASE_PKG_FILE="/home/travis/build/VitensTC/VIPhreeqc/build/lib/libiphreeqc-3.3.7.dylib"; fi
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then export DEPLOY_PKG_FILE="viphreeqc.dylib"; fi
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then export RELEASE_PKG_FILE="/home/travis/build/VitensTC/VIPhreeqc/build/lib/libiphreeqc-3.3.7.so"; fi
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then export DEPLOY_PKG_FILE="viphreeqc.so"; fi

###
# Copy your source files to a deployment directory
echo "Copying built files"
cp $RELEASE_PKG_FILE _deploy/phreeqpython/lib/$DEPLOY_PKG_FILE
###

# Move into deployment directory
cd _deploy

echo "Committing and pushing to GH"

git config user.name "Travis CI"
git config user.email "travis@vitens.nl"

# Commit changes, allowing empty changes (when unchanged)
git add -A
git commit --allow-empty -m "Deploying build for $CURRENT_COMMIT" || exit 1

# Push to branch
git push origin master > /dev/null 2>&1 || exit 1

echo "Pushed deployment successfully"
exit 0
