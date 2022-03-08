# opssat-cloud-detection
Cloud detection repository for OpsSat.

## Build with CMake:

1. Navigate to `bin`
2. Generate the CMake files with `cmake ../`
3. Once the makefiles are generated, build with `cmake --build .`

## Run random forest EM test:

1. Build binaries (see "Build with CMake" section)
2. Run `bash random_forest.sh` (or just run `random_forest.sh` if using bash as your shell)