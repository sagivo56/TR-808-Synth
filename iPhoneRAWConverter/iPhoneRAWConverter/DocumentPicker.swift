import SwiftUI
import UniformTypeIdentifiers

struct DocumentPicker: UIViewControllerRepresentable {
    let onPick: ([URL]) -> Void
    @Environment(\.dismiss) private var dismiss

    private static let rawContentTypes: [UTType] = {
        let extensions = ["arw","raw","cr2","cr3","nef","nrw","orf","rw2",
                          "pef","dng","raf","srw","x3f","3fr","mef","rwl","erf","kdc","dcr"]
        var types: [UTType] = extensions.compactMap { UTType(filenameExtension: $0) }
        types.append(.data)
        var seen = Set<String>()
        return types.filter { seen.insert($0.identifier).inserted }
    }()

    func makeCoordinator() -> Coordinator { Coordinator(self) }

    func makeUIViewController(context: Context) -> UIDocumentPickerViewController {
        let picker = UIDocumentPickerViewController(
            forOpeningContentTypes: Self.rawContentTypes,
            asCopy: true
        )
        picker.allowsMultipleSelection = true
        picker.shouldShowFileExtensions = true
        picker.delegate = context.coordinator
        return picker
    }

    func updateUIViewController(_ vc: UIDocumentPickerViewController, context: Context) {}

    class Coordinator: NSObject, UIDocumentPickerDelegate {
        let parent: DocumentPicker
        init(_ parent: DocumentPicker) { self.parent = parent }

        func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentsAt urls: [URL]) {
            parent.onPick(urls)
            parent.dismiss()
        }

        func documentPickerWasCancelled(_ controller: UIDocumentPickerViewController) {
            parent.dismiss()
        }
    }
}

struct ShareSheet: UIViewControllerRepresentable {
    let items: [Any]
    @Environment(\.dismiss) private var dismiss

    func makeUIViewController(context: Context) -> UIActivityViewController {
        let vc = UIActivityViewController(activityItems: items, applicationActivities: nil)
        vc.completionWithItemsHandler = { _, _, _, _ in }
        return vc
    }

    func updateUIViewController(_ vc: UIActivityViewController, context: Context) {}
}
