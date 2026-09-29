// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "PronunciationEngineCore",
    platforms: [
        .iOS(.v18)
    ],
    products: [
        .library(name: "PronunciationEngineCore", targets: ["PronunciationEngineCore"])
    ],
    targets: [
        // Microsoft's signed/released C/C++ iOS archive used by its official
        // Swift Package Manager wrapper. Pin the checksum so dependency bytes
        // cannot drift under the build.
        .binaryTarget(
            name: "onnxruntime",
            url: "https://download.onnxruntime.ai/pod-archive-onnxruntime-c-1.24.2.zip",
            checksum: "f7100a992d2a8135168c8afd831e6a58b465349101982aa58b3e11d36e600b54"
        ),
        .target(
            name: "PronunciationEngineCore",
            dependencies: ["onnxruntime"],
            path: ".",
            exclude: [
                ".gitignore",
                "CMakeLists.txt",
                "Harness",
                "README.md",
                "Resources",
                "Tests",
                "THIRD_PARTY_NOTICES.md",
                "model"
            ],
            sources: ["src"],
            publicHeadersPath: "include",
            cxxSettings: [
                .define("PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME"),
                .headerSearchPath("src"),
                .headerSearchPath("third_party")
            ]
        )
    ],
    cxxLanguageStandard: .cxx17
)
