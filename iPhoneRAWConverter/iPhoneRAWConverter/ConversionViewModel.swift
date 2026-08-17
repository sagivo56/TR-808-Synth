import SwiftUI
import Photos

struct ConvertedFile: Identifiable {
    let id = UUID()
    let originalName: String
    let data: Data
    let format: OutputFormat
    let thumbnail: UIImage?

    var fileName: String {
        let base = (originalName as NSString).deletingPathExtension
        return "\(base).\(format.fileExtension)"
    }

    var fileSize: String {
        ByteCountFormatter.string(fromByteCount: Int64(data.count), countStyle: .file)
    }
}

@MainActor
class ConversionViewModel: ObservableObject {
    @Published var selectedURLs: [URL] = []
    @Published var convertedFiles: [ConvertedFile] = []
    @Published var isConverting = false
    @Published var progress: Double = 0
    @Published var outputFormat: OutputFormat = .jpeg
    @Published var quality: Float = 0.88
    @Published var errorMessage: String?
    @Published var showingDocumentPicker = false
    @Published var showingShareSheet = false
    @Published var savedToPhotos = false

    var shareItems: [Any] = []

    func addFiles(_ urls: [URL]) {
        let fresh = urls.filter { new in !selectedURLs.contains(new) }
        selectedURLs.append(contentsOf: fresh)
    }

    func removeFile(at offsets: IndexSet) {
        selectedURLs.remove(atOffsets: offsets)
    }

    func clearAll() {
        selectedURLs = []
        convertedFiles = []
        errorMessage = nil
    }

    func convert() async {
        guard !selectedURLs.isEmpty else { return }
        isConverting = true
        convertedFiles = []
        progress = 0
        errorMessage = nil

        let urls = selectedURLs
        let format = outputFormat
        let q = quality
        let total = Double(urls.count)
        var results: [ConvertedFile] = []
        var errors: [String] = []

        await withTaskGroup(of: (Int, Result<ConvertedFile, Error>).self) { group in
            for (i, url) in urls.enumerated() {
                group.addTask {
                    do {
                        let data = try RAWProcessor.convert(url: url, to: format, quality: q)
                        let thumb = RAWProcessor.thumbnail(data: data)
                        let file = ConvertedFile(
                            originalName: url.lastPathComponent,
                            data: data,
                            format: format,
                            thumbnail: thumb
                        )
                        return (i, .success(file))
                    } catch {
                        return (i, .failure(error))
                    }
                }
            }

            var completed = 0
            for await (_, result) in group {
                completed += 1
                progress = Double(completed) / total
                switch result {
                case .success(let file):
                    results.append(file)
                case .failure(let err):
                    errors.append(err.localizedDescription)
                }
            }
        }

        results.sort { $0.originalName < $1.originalName }
        convertedFiles = results

        if !errors.isEmpty {
            errorMessage = errors.joined(separator: "\n")
        }
        isConverting = false
    }

    func saveToPhotos() async {
        let status = await PHPhotoLibrary.requestAuthorization(for: .addOnly)
        guard status == .authorized || status == .limited else {
            errorMessage = "נדרשת הרשאה לגישה לגלריית התמונות"
            return
        }
        do {
            try await PHPhotoLibrary.shared().performChanges {
                for file in self.convertedFiles {
                    guard let img = UIImage(data: file.data) else { continue }
                    PHAssetChangeRequest.creationRequestForAsset(from: img)
                }
            }
            savedToPhotos = true
        } catch {
            errorMessage = "שגיאה בשמירה לגלריה: \(error.localizedDescription)"
        }
    }

    func prepareShare() {
        shareItems = convertedFiles.compactMap { file -> URL? in
            let tmp = FileManager.default.temporaryDirectory.appendingPathComponent(file.fileName)
            try? file.data.write(to: tmp)
            return tmp
        }
        showingShareSheet = true
    }
}
