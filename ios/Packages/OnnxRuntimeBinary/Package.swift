// swift-tools-version: 5.9

import PackageDescription

// Official ONNX Runtime 1.30.0 Apple C/C++ binary, distributed by Microsoft.
// A tiny local package lets the generated Xcode project use Swift Package
// Manager while consuming the same archive published for CocoaPods.
let package = Package(
    name: "OnnxRuntimeBinary",
    platforms: [.iOS(.v15)],
    products: [
        .library(name: "onnxruntime", targets: ["onnxruntime"])
    ],
    targets: [
        .binaryTarget(
            name: "onnxruntime",
            url: "https://download.onnxruntime.ai/pod-archive-onnxruntime-c-1.30.0.zip",
            checksum: "e6f1670c14406fd9f082bb400ab197a9b0a9646058ca6366e440642e2b54a2ea"
        )
    ]
)
