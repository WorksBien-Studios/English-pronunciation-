#!/usr/bin/env python3
"""Generate ios/EnglishPronunciationCoach.xcodeproj from the source tree.

The project file is generated (not hand-edited) so it always lists exactly the files on disk.
Format follows the hand-written project in lrodeveloperr/ios-18-shell (objectVersion 56).

    python3 scripts/generate_xcodeproj.py          # write the project
    python3 scripts/generate_xcodeproj.py --check  # fail if the committed project is stale
"""
import hashlib
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
IOS = ROOT / "ios"
APP_NAME = "EnglishPronunciationCoach"
TEST_NAME = "EnglishPronunciationCoachTests"
PROJECT = IOS / f"{APP_NAME}.xcodeproj"
BUNDLE_ID = "com.worksbienstudios.englishpronunciationcoach"
SHELL_URL = "https://github.com/lrodeveloperr/ios-18-shell"
SHELL_REVISION = "c082e90f9fc92970ef127792dfba2dd9cdffd710"
ORT_PACKAGE_PATH = "Packages/OnnxRuntimeBinary"
MIC_TEXT = "発音を録音し、端末上で音ごとのフィードバックを表示するためにマイクを使用します。録音は分析のために外部サーバーへ送信されません。"


def ident(*parts):
    return hashlib.md5("|".join(parts).encode("utf-8")).hexdigest()[:24].upper()


def file_type(path):
    suffix = path.suffix
    return {
        ".swift": "sourcecode.swift",
        ".cpp": "sourcecode.cpp.cpp",
        ".h": "sourcecode.c.h",
        ".json": "text.json",
        ".xcprivacy": "text.xml",
        ".plist": "text.plist.xml",
        ".xcassets": "folder.assetcatalog",
    }.get(suffix, "text")


def collect(directory):
    """Files (and .xcassets folders) below a directory, as sorted relative paths."""
    result = []
    for path in sorted(directory.rglob("*")):
        if any(part.endswith(".xcassets") for part in path.relative_to(directory).parts[:-1]):
            continue  # inside an asset catalog
        if path.is_dir():
            if path.suffix == ".xcassets":
                result.append(path.relative_to(directory))
            continue
        if path.name == ".DS_Store":
            continue
        if path.name == "model_q4f16.onnx":
            continue  # large ignored artifact has one stable, explicit project reference
        result.append(path.relative_to(directory))
    return result


class Builder:
    def __init__(self):
        self.build_files = []
        self.file_refs = []
        self.groups = []

    def add_group(self, key, name, children, path=None):
        gid = ident("group", key)
        body = f"isa = PBXGroup; children = ({', '.join(children)}); "
        if path is not None:
            body += f"path = {quote(path)}; "
        elif name:
            body += f"name = {quote(name)}; "
        body += 'sourceTree = "<group>"; '
        self.groups.append(f"\t\t{gid} /* {name or path} */ = {{{body}}};")
        return gid

    def add_tree(self, key, directory, prefix):
        """Create groups/file refs mirroring the directory. Returns (group id, files)."""
        entries = collect(directory)
        files = []
        subdirs = {}
        for rel in entries:
            if len(rel.parts) == 1 or rel.suffix == ".xcassets" and len(rel.parts) == 1:
                files.append(rel)
            else:
                subdirs.setdefault(rel.parts[0], []).append(rel)
        children = []
        sources, resources = [], []
        for rel in files:
            fid = ident("file", prefix, str(rel))
            kind = file_type(rel)
            self.file_refs.append(
                f"\t\t{fid} /* {rel.name} */ = {{isa = PBXFileReference; lastKnownFileType = {kind}; "
                f"path = {quote(rel.name)}; sourceTree = \"<group>\"; }};"
            )
            children.append(f"{fid} /* {rel.name} */")
            if rel.suffix in (".swift", ".cpp", ".mm"):
                sources.append((fid, rel.name))
            elif rel.name == "Info.plist":
                pass  # merged through INFOPLIST_FILE, never copied as a resource
            elif rel.suffix not in (".h", ".hpp"):
                resources.append((fid, rel.name))
        for name in sorted(subdirs):
            sub_gid, sub_sources, sub_resources = self.add_subtree(f"{key}/{name}", directory / name, f"{prefix}/{name}", name)
            children.append(f"{sub_gid} /* {name} */")
            sources.extend(sub_sources)
            resources.extend(sub_resources)
        return children, sources, resources

    def add_subtree(self, key, directory, prefix, name):
        children, sources, resources = self.add_tree(key, directory, prefix)
        gid = self.add_group(key, name, children, path=name)
        return gid, sources, resources


def quote(value):
    # Old-style plist strings may only be unquoted when they are plain ASCII words.
    if value and all(c.isascii() and (c.isalnum() or c in "._/") for c in value):
        return value
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def build_file(kind_phase, fid, name, product_ref=None):
    bid = ident("buildfile", kind_phase, fid)
    return bid, f"\t\t{bid} /* {name} in {kind_phase} */ = {{isa = PBXBuildFile; fileRef = {fid} /* {name} */; }};"


def render():
    b = Builder()
    app_children, app_sources, app_resources = b.add_tree("app", IOS / APP_NAME, "app")
    test_children, test_sources, _ = b.add_tree("tests", IOS / TEST_NAME, "tests")
    engine_children, engine_sources, _ = b.add_tree(
        "engine-src", ROOT / "pronunciation-engine" / "src", "engine-src")
    engine_include_children, _, _ = b.add_tree(
        "engine-include", ROOT / "pronunciation-engine" / "include", "engine-include")
    app_group = b.add_group("app-root", APP_NAME, app_children, path=APP_NAME)
    test_group = b.add_group("tests-root", TEST_NAME, test_children, path=TEST_NAME)
    engine_src_group = b.add_group(
        "engine-src-root", "PronunciationEngine Sources", engine_children,
        path="../pronunciation-engine/src")
    engine_include_group = b.add_group(
        "engine-include-root", "PronunciationEngine Public API", engine_include_children,
        path="../pronunciation-engine/include")
    app_sources.extend(engine_sources)

    app_product = ident("product", "app")
    test_product = ident("product", "tests")
    products_group = ident("group", "products")
    main_group = ident("group", "main")
    shell_package_ref = ident("package", "shell")
    shell_package_dep = ident("packagedep", "shell")
    ort_package_ref = ident("package", "onnxruntime")
    ort_package_dep = ident("packagedep", "onnxruntime")
    project_id = ident("project")
    app_target = ident("target", "app")
    test_target = ident("target", "tests")

    build_lines, app_src_ids, app_res_ids, test_src_ids = [], [], [], []
    for fid, name in app_sources:
        bid, line = build_file("Sources", fid, name)
        build_lines.append(line); app_src_ids.append(f"{bid} /* {name} in Sources */")
    for fid, name in app_resources:
        bid, line = build_file("Resources", fid, name)
        build_lines.append(line); app_res_ids.append(f"{bid} /* {name} in Resources */")
    for fid, name in test_sources:
        bid, line = build_file("TestSources", fid, name)
        build_lines.append(line); test_src_ids.append(f"{bid} /* {name} in Sources */")
    shell_build = ident("buildfile", "shell")
    ort_build = ident("buildfile", "onnxruntime")
    build_lines.append(f"\t\t{shell_build} /* iOS18Shell in Frameworks */ = {{isa = PBXBuildFile; productRef = {shell_package_dep} /* iOS18Shell */; }};")
    build_lines.append(f"\t\t{ort_build} /* onnxruntime in Frameworks */ = {{isa = PBXBuildFile; productRef = {ort_package_dep} /* onnxruntime */; }};")

    engine_resources_ref = ident("file", "engine-resources-folder")
    model_ref = ident("file", "model_q4f16.onnx")
    b.file_refs.extend([
        f"\t\t{engine_resources_ref} /* PronunciationEngineResources */ = {{isa = PBXFileReference; lastKnownFileType = folder; name = PronunciationEngineResources; path = ../pronunciation-engine/Resources; sourceTree = \"<group>\"; }};",
        f"\t\t{model_ref} /* model_q4f16.onnx */ = {{isa = PBXFileReference; lastKnownFileType = file; path = {APP_NAME}/Resources/model_q4f16.onnx; sourceTree = SOURCE_ROOT; }};",
    ])
    engine_resources_build, line = build_file("Resources", engine_resources_ref, "PronunciationEngineResources")
    build_lines.append(line)
    model_build, line = build_file("Resources", model_ref, "model_q4f16.onnx")
    build_lines.append(line)
    app_res_ids.extend([
        f"{engine_resources_build} /* PronunciationEngineResources in Resources */",
        f"{model_build} /* model_q4f16.onnx in Resources */",
    ])

    app_phases = {k: ident("phase", "app", k) for k in ("sources", "frameworks", "resources", "sign-model")}
    test_phases = {k: ident("phase", "tests", k) for k in ("sources", "frameworks")}
    proxy = ident("proxy")
    dependency = ident("dependency")
    cfg = {k: ident("cfg", k) for k in ("proj-debug", "proj-release", "app-debug", "app-release", "test-debug", "test-release")}
    lists = {k: ident("cfglist", k) for k in ("proj", "app", "test")}

    def section(name, lines):
        return f"/* Begin {name} section */\n" + "\n".join(lines) + f"\n/* End {name} section */\n"

    out = ["// !$*UTF8*$!", "{", "\tarchiveVersion = 1;", "\tclasses = {};", "\tobjectVersion = 56;", "\tobjects = {", ""]
    out.append(section("PBXBuildFile", sorted(build_lines)))
    out.append(section("PBXContainerItemProxy", [
        f"\t\t{proxy} /* PBXContainerItemProxy */ = {{isa = PBXContainerItemProxy; containerPortal = {project_id} /* Project object */; proxyType = 1; remoteGlobalIDString = {app_target}; remoteInfo = {APP_NAME}; }};"]))
    out.append(section("PBXFileReference", sorted(b.file_refs) + [
        f"\t\t{app_product} /* {APP_NAME}.app */ = {{isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = {APP_NAME}.app; sourceTree = BUILT_PRODUCTS_DIR; }};",
        f"\t\t{test_product} /* {TEST_NAME}.xctest */ = {{isa = PBXFileReference; explicitFileType = wrapper.cfbundle; includeInIndex = 0; path = {TEST_NAME}.xctest; sourceTree = BUILT_PRODUCTS_DIR; }};"]))
    out.append(section("PBXFrameworksBuildPhase", [
        f"\t\t{app_phases['frameworks']} /* Frameworks */ = {{isa = PBXFrameworksBuildPhase; buildActionMask = 2147483647; files = ({shell_build} /* iOS18Shell in Frameworks */, {ort_build} /* onnxruntime in Frameworks */); runOnlyForDeploymentPostprocessing = 0; }};",
        f"\t\t{test_phases['frameworks']} /* Frameworks */ = {{isa = PBXFrameworksBuildPhase; buildActionMask = 2147483647; files = (); runOnlyForDeploymentPostprocessing = 0; }};"]))
    b.groups.append(f"\t\t{main_group} = {{isa = PBXGroup; children = ({app_group} /* {APP_NAME} */, {test_group} /* {TEST_NAME} */, {engine_src_group} /* PronunciationEngine Sources */, {engine_include_group} /* PronunciationEngine Public API */, {engine_resources_ref} /* PronunciationEngineResources */, {model_ref} /* model_q4f16.onnx */, {products_group} /* Products */); sourceTree = \"<group>\"; }};")
    b.groups.append(f"\t\t{products_group} /* Products */ = {{isa = PBXGroup; children = ({app_product} /* {APP_NAME}.app */, {test_product} /* {TEST_NAME}.xctest */); name = Products; sourceTree = \"<group>\"; }};")
    out.append(section("PBXGroup", sorted(b.groups)))
    out.append(section("PBXNativeTarget", [
        f"\t\t{app_target} /* {APP_NAME} */ = {{isa = PBXNativeTarget; buildConfigurationList = {lists['app']} /* Build configuration list for PBXNativeTarget \"{APP_NAME}\" */; buildPhases = ({app_phases['sources']} /* Sources */, {app_phases['frameworks']} /* Frameworks */, {app_phases['resources']} /* Resources */, {app_phases['sign-model']} /* Sign bundled ONNX model */); buildRules = (); dependencies = (); name = {APP_NAME}; packageProductDependencies = ({shell_package_dep} /* iOS18Shell */, {ort_package_dep} /* onnxruntime */); productName = {APP_NAME}; productReference = {app_product} /* {APP_NAME}.app */; productType = \"com.apple.product-type.application\"; }};",
        f"\t\t{test_target} /* {TEST_NAME} */ = {{isa = PBXNativeTarget; buildConfigurationList = {lists['test']} /* Build configuration list for PBXNativeTarget \"{TEST_NAME}\" */; buildPhases = ({test_phases['sources']} /* Sources */, {test_phases['frameworks']} /* Frameworks */); buildRules = (); dependencies = ({dependency} /* PBXTargetDependency */); name = {TEST_NAME}; productName = {TEST_NAME}; productReference = {test_product} /* {TEST_NAME}.xctest */; productType = \"com.apple.product-type.bundle.unit-test\"; }};"]))
    out.append(section("PBXProject", [
        f"\t\t{project_id} /* Project object */ = {{isa = PBXProject; attributes = {{BuildIndependentTargetsInParallel = 1; LastUpgradeCheck = 1640; TargetAttributes = {{{app_target} = {{CreatedOnToolsVersion = 16.4; }}; {test_target} = {{CreatedOnToolsVersion = 16.4; TestTargetID = {app_target}; }}; }}; }}; buildConfigurationList = {lists['proj']} /* Build configuration list for PBXProject */; compatibilityVersion = \"Xcode 14.0\"; developmentRegion = ja; hasScannedForEncodings = 0; knownRegions = (ja, en, Base); mainGroup = {main_group}; packageReferences = ({shell_package_ref} /* XCRemoteSwiftPackageReference \"ios-18-shell\" */, {ort_package_ref} /* XCLocalSwiftPackageReference \"OnnxRuntimeBinary\" */); productRefGroup = {products_group} /* Products */; projectDirPath = \"\"; projectRoot = \"\"; targets = ({app_target} /* {APP_NAME} */, {test_target} /* {TEST_NAME} */); }};"]))
    out.append(section("PBXResourcesBuildPhase", [
        f"\t\t{app_phases['resources']} /* Resources */ = {{isa = PBXResourcesBuildPhase; buildActionMask = 2147483647; files = ({', '.join(app_res_ids)}); runOnlyForDeploymentPostprocessing = 0; }};"]))
    model_sign_script = (
        'if [ "${CODE_SIGNING_ALLOWED}" = YES ] && [ "${EFFECTIVE_PLATFORM_NAME}" = -iphonesimulator ]; then '
        'resource_root="${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}"; '
        'for name in model_q4f16.onnx PrivacyInfo.xcprivacy stages.json Assets.car; do '
        'file="$resource_root/$name"; if [ -f "$file" ]; then /usr/bin/codesign --force --sign - --timestamp=none "$file"; fi; '
        'done; '
        'if [ -d "$resource_root/Resources" ]; then find "$resource_root/Resources" -type f -exec /usr/bin/codesign --force --sign - --timestamp=none {} \\; ; fi; '
        'fi'
    )
    out.append(section("PBXShellScriptBuildPhase", [
        f"\t\t{app_phases['sign-model']} /* Sign bundled ONNX model */ = {{isa = PBXShellScriptBuildPhase; buildActionMask = 2147483647; files = (); inputFileListPaths = (); inputPaths = (); name = \"Sign bundled ONNX model\"; outputFileListPaths = (); outputPaths = (); runOnlyForDeploymentPostprocessing = 0; shellPath = /bin/sh; shellScript = {quote(model_sign_script)}; }};"
    ]))
    out.append(section("PBXSourcesBuildPhase", [
        f"\t\t{app_phases['sources']} /* Sources */ = {{isa = PBXSourcesBuildPhase; buildActionMask = 2147483647; files = ({', '.join(app_src_ids)}); runOnlyForDeploymentPostprocessing = 0; }};",
        f"\t\t{test_phases['sources']} /* Sources */ = {{isa = PBXSourcesBuildPhase; buildActionMask = 2147483647; files = ({', '.join(test_src_ids)}); runOnlyForDeploymentPostprocessing = 0; }};"]))
    out.append(section("PBXTargetDependency", [
        f"\t\t{dependency} /* PBXTargetDependency */ = {{isa = PBXTargetDependency; target = {app_target} /* {APP_NAME} */; targetProxy = {proxy} /* PBXContainerItemProxy */; }};"]))

    project_common = "ALWAYS_SEARCH_USER_PATHS = NO; CLANG_ENABLE_MODULES = YES; CLANG_CXX_LANGUAGE_STANDARD = \"c++17\"; IPHONEOS_DEPLOYMENT_TARGET = 18.0; SDKROOT = iphoneos; SWIFT_VERSION = 5.0; "
    debug_extra = "DEBUG_INFORMATION_FORMAT = dwarf; ENABLE_TESTABILITY = YES; ONLY_ACTIVE_ARCH = YES; SWIFT_ACTIVE_COMPILATION_CONDITIONS = \"DEBUG $(inherited)\"; SWIFT_OPTIMIZATION_LEVEL = \"-Onone\"; "
    release_extra = "DEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\"; ENABLE_NS_ASSERTIONS = NO; SWIFT_COMPILATION_MODE = wholemodule; SWIFT_OPTIMIZATION_LEVEL = \"-O\"; VALIDATE_PRODUCT = YES; "
    app_settings = (
        "ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon; CODE_SIGN_STYLE = Automatic; CURRENT_PROJECT_VERSION = 1; ENABLE_PREVIEWS = YES; GENERATE_INFOPLIST_FILE = YES; "
        f"INFOPLIST_FILE = {APP_NAME}/Resources/Info.plist; "
        f"INFOPLIST_KEY_CFBundleDisplayName = {quote('英語発音コーチ')}; INFOPLIST_KEY_LSApplicationCategoryType = \"public.app-category.education\"; "
        f"INFOPLIST_KEY_NSMicrophoneUsageDescription = {quote(MIC_TEXT)}; "
        "INFOPLIST_KEY_UIApplicationSceneManifest_Generation = YES; INFOPLIST_KEY_UILaunchScreen_Generation = YES; "
        "INFOPLIST_KEY_UISupportedInterfaceOrientations_iPad = \"UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight\"; "
        "INFOPLIST_KEY_UISupportedInterfaceOrientations_iPhone = UIInterfaceOrientationPortrait; "
        f"IPHONEOS_DEPLOYMENT_TARGET = 18.0; MARKETING_VERSION = 1.0; PRODUCT_BUNDLE_IDENTIFIER = {BUNDLE_ID}; PRODUCT_NAME = \"$(TARGET_NAME)\"; "
        "GCC_PREPROCESSOR_DEFINITIONS = \"$(inherited) PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME=1\"; "
        "OTHER_LDFLAGS = \"$(inherited) -framework Network\"; "
        "HEADER_SEARCH_PATHS = \"$(inherited) $(PROJECT_DIR)/../pronunciation-engine/include $(PROJECT_DIR)/../pronunciation-engine/src $(PROJECT_DIR)/../pronunciation-engine/third_party\"; "
        f"SWIFT_OBJC_BRIDGING_HEADER = {APP_NAME}/EngineBridge/PronunciationEngine-Bridging-Header.h; "
        "SUPPORTED_PLATFORMS = \"iphoneos iphonesimulator\"; SUPPORTS_MACCATALYST = NO; SWIFT_EMIT_LOC_STRINGS = YES; SWIFT_VERSION = 5.0; TARGETED_DEVICE_FAMILY = \"1,2\"; "
    )
    test_settings = (
        f"BUNDLE_LOADER = \"$(TEST_HOST)\"; CODE_SIGN_STYLE = Automatic; GENERATE_INFOPLIST_FILE = YES; IPHONEOS_DEPLOYMENT_TARGET = 18.0; "
        f"PRODUCT_BUNDLE_IDENTIFIER = {BUNDLE_ID}.tests; PRODUCT_NAME = \"$(TARGET_NAME)\"; SWIFT_VERSION = 5.0; TARGETED_DEVICE_FAMILY = \"1,2\"; "
        f"TEST_HOST = \"$(BUILT_PRODUCTS_DIR)/{APP_NAME}.app/$(BUNDLE_EXECUTABLE_FOLDER_PATH)/{APP_NAME}\"; "
    )

    def conf(key, settings, name):
        return f"\t\t{cfg[key]} /* {name} */ = {{isa = XCBuildConfiguration; buildSettings = {{{settings}}}; name = {name}; }};"

    out.append(section("XCBuildConfiguration", [
        conf("proj-debug", project_common + debug_extra, "Debug"),
        conf("proj-release", project_common + release_extra, "Release"),
        conf("app-debug", app_settings, "Debug"),
        conf("app-release", app_settings, "Release"),
        conf("test-debug", test_settings, "Debug"),
        conf("test-release", test_settings, "Release")]))

    def config_list(key, what, debug, release):
        return (f"\t\t{lists[key]} /* Build configuration list for {what} */ = {{isa = XCConfigurationList; "
                f"buildConfigurations = ({cfg[debug]} /* Debug */, {cfg[release]} /* Release */); defaultConfigurationIsVisible = 0; defaultConfigurationName = Release; }};")

    out.append(section("XCConfigurationList", [
        config_list("proj", "PBXProject", "proj-debug", "proj-release"),
        config_list("app", f"PBXNativeTarget \"{APP_NAME}\"", "app-debug", "app-release"),
        config_list("test", f"PBXNativeTarget \"{TEST_NAME}\"", "test-debug", "test-release")]))
    out.append(section("XCRemoteSwiftPackageReference", [
        f"\t\t{shell_package_ref} /* XCRemoteSwiftPackageReference \"ios-18-shell\" */ = {{isa = XCRemoteSwiftPackageReference; repositoryURL = \"{SHELL_URL}\"; requirement = {{kind = revision; revision = {SHELL_REVISION}; }}; }};"]))
    out.append(section("XCLocalSwiftPackageReference", [
        f"\t\t{ort_package_ref} /* XCLocalSwiftPackageReference \"OnnxRuntimeBinary\" */ = {{isa = XCLocalSwiftPackageReference; relativePath = {ORT_PACKAGE_PATH}; }};"]))
    out.append(section("XCSwiftPackageProductDependency", [
        f"\t\t{shell_package_dep} /* iOS18Shell */ = {{isa = XCSwiftPackageProductDependency; package = {shell_package_ref} /* XCRemoteSwiftPackageReference \"ios-18-shell\" */; productName = iOS18Shell; }};",
        f"\t\t{ort_package_dep} /* onnxruntime */ = {{isa = XCSwiftPackageProductDependency; package = {ort_package_ref} /* XCLocalSwiftPackageReference \"OnnxRuntimeBinary\" */; productName = onnxruntime; }};",
    ]))
    out.append("\t};")
    out.append(f"\trootObject = {project_id} /* Project object */;")
    out.append("}")

    scheme = f'''<?xml version="1.0" encoding="UTF-8"?>
<Scheme LastUpgradeVersion="1640" version="1.3">
   <BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES">
      <BuildActionEntries>
         <BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES">
            <BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{app_target}" BuildableName="{APP_NAME}.app" BlueprintName="{APP_NAME}" ReferencedContainer="container:{APP_NAME}.xcodeproj"/>
         </BuildActionEntry>
      </BuildActionEntries>
   </BuildAction>
   <TestAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" shouldUseLaunchSchemeArgsEnv="YES">
      <Testables>
         <TestableReference skipped="NO">
            <BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{test_target}" BuildableName="{TEST_NAME}.xctest" BlueprintName="{TEST_NAME}" ReferencedContainer="container:{APP_NAME}.xcodeproj"/>
         </TestableReference>
      </Testables>
   </TestAction>
   <LaunchAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" launchStyle="0" useCustomWorkingDirectory="NO" ignoresPersistentStateOnLaunch="NO" debugDocumentVersioning="YES" debugServiceExtension="internal" allowLocationSimulation="YES">
      <BuildableProductRunnable runnableDebuggingMode="0">
         <BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{app_target}" BuildableName="{APP_NAME}.app" BlueprintName="{APP_NAME}" ReferencedContainer="container:{APP_NAME}.xcodeproj"/>
      </BuildableProductRunnable>
   </LaunchAction>
   <ArchiveAction buildConfiguration="Release" revealArchiveInOrganizer="YES"/>
</Scheme>
'''
    workspace = '<?xml version="1.0" encoding="UTF-8"?>\n<Workspace\n   version = "1.0">\n   <FileRef\n      location = "self:">\n   </FileRef>\n</Workspace>\n'
    return {
        PROJECT / "project.pbxproj": "\n".join(out) + "\n",
        PROJECT / "xcshareddata" / "xcschemes" / f"{APP_NAME}.xcscheme": scheme,
        PROJECT / "project.xcworkspace" / "contents.xcworkspacedata": workspace,
    }


def validate_openstep(text):
    """Strict old-style plist parser: raises SystemExit on anything Xcode would reject."""
    i, n = 0, len(text)

    def fail(message):
        line = text.count("\n", 0, i) + 1
        raise SystemExit(f"project.pbxproj parse error (line {line}): {message}")

    def ws():
        nonlocal i
        while i < n:
            if text[i].isspace():
                i += 1
            elif text.startswith("/*", i):
                j = text.find("*/", i)
                if j < 0:
                    fail("unterminated comment")
                i = j + 2
            elif text.startswith("//", i):
                j = text.find("\n", i)
                i = n if j < 0 else j + 1
            else:
                break

    def string():
        nonlocal i
        if text[i] == '"':
            i += 1
            while text[i] != '"':
                i += 2 if text[i] == "\\" else 1
            i += 1
            return
        j = i
        while i < n and text[i].isascii() and (text[i].isalnum() or text[i] in "_$/:.-"):
            i += 1
        if i == j:
            fail(f"unquoted or unexpected text {text[i:i + 30]!r}")

    def value():
        nonlocal i
        ws()
        if text[i] == "{":
            i += 1
            while True:
                ws()
                if text[i] == "}":
                    i += 1
                    return
                string(); ws()
                if text[i] != "=":
                    fail("expected '='")
                i += 1
                value(); ws()
                if text[i] != ";":
                    fail("expected ';'")
                i += 1
        elif text[i] == "(":
            i += 1
            while True:
                ws()
                if text[i] == ")":
                    i += 1
                    return
                value(); ws()
                if text[i] == ",":
                    i += 1
        else:
            string()

    value(); ws()
    if i != n:
        fail("trailing content")


def main():
    files = render()
    validate_openstep(files[PROJECT / "project.pbxproj"])
    if "--check" in sys.argv:
        stale = [str(p.relative_to(ROOT)) for p, text in files.items() if not p.exists() or p.read_text(encoding="utf-8") != text]
        if stale:
            sys.exit("Xcode project is stale; run scripts/generate_xcodeproj.py: " + ", ".join(stale))
        print("Xcode project is up to date.")
        return
    for path, text in files.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        print("wrote", path.relative_to(ROOT))


if __name__ == "__main__":
    main()
