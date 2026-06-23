# Building TR-808 Synth for iOS

The DSP/engine and the JUCE codebase are portable, so the same source builds an
**iOS Standalone app + AUv3** (works inside GarageBand, AUM, Cubasis, etc.).

> ⚠️ **A Mac is required.** Apple's compiler, code-signing and on-device deploy run
> only through **Xcode (macOS)** — you cannot build iOS from Windows. Options:
> a cloud Mac (MacinCloud / MacStadium / AWS EC2 Mac), a borrowed Mac, or a Mac mini.
> An **Apple Developer account** is needed to run on a device / distribute
> (a free Apple ID gives 7-day on-device provisioning for testing).

> ⚠️ **UI is still desktop-sized.** The current layout targets a ~1180px window; on a
> phone it will be cramped. A touch/responsive mobile UI is the next work item
> (the engine itself is ready). On an iPad it's already more usable.

## On the Mac

1. Install **Xcode** (App Store) and **CMake** (`brew install cmake`).
2. Get the source (clone the repo, or unzip `TR-808-Synth-source.zip`).
3. Configure an Xcode project for iOS:

   ```bash
   cmake -B build-ios -G Xcode \
     -DCMAKE_SYSTEM_NAME=iOS \
     -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
     -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=YOUR_TEAM_ID
   ```
   (`YOUR_TEAM_ID` is from your Apple Developer account → Membership. JUCE is fetched
   automatically on first configure.)

4. Open `build-ios/TR808Synth.xcodeproj` in Xcode (or `cmake --build build-ios --config Release`).
5. In Xcode: pick the **TR-808 Synth - Standalone Plugin** scheme, select your device,
   set **Signing & Capabilities → Team**, then **Run**. The AUv3 is bundled in the app
   and appears in AUv3 hosts after the app is installed once.

## Notes
- iOS formats are set automatically (`Standalone AUv3`) when `CMAKE_SYSTEM_NAME=iOS`
  — see the `IOS` block in `CMakeLists.txt`.
- Orientation/full-screen/background-audio Info.plist keys are already set in
  `juce_add_plugin`.
- For the App Store / TestFlight you need the paid Apple Developer program and an
  app record in App Store Connect.
