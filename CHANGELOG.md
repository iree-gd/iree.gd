# Change Log

`iree.gd`, [IREE](https://github.com/openxla/iree) runtime in Godot through GDExtension, a mission to run machine learning model (e.g. Tensorflow lite) natively in Godot.

This is the change log of `iree.gd`.

## [v0.0.1-alpha6] Upgrade IREE

- Upgrade IREE version.
- Add pose detection example.
- Remove synchronous execution.

## [v0.0.1-alpha5] Add asynchronous execution

- Add asynchronous execution.
- Update IREE to `candidate-20240123.779`.
- Add CI/CD for Mac OS and iOS.

## [v0.0.1-alpha4] Improve demo ease of use

- Defer context initialization to raise error only on `IREEModule::run_module`.
- Detect platform to use the correct bytecode.
- Set sample's main scene.

## [v0.0.1-alpha3] Quick import `.vmfb` files

- Import `.vmfb` files directly. (Thanks to [Dragos Daian](https://github.com/Ughuuu))
- Fix Linux ARM build.

## [v0.0.1-alpha2] Update sample bytecodes

- Improve logging `IREE` error.
- Update sample bytecodes.

## [v0.0.1-alpha1] Windows build fix

- Export `.dll` instead of `lib` for windows build.

## [v0.0.1-alpha0] The very beginning

- Add `IREETensor` and `IREEModule` for executing machine learning models.
- Add a sample for people to try out.
- Add builds for Android, Linux and Windows.
