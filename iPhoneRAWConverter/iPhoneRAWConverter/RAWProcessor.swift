import CoreImage
import UIKit

enum OutputFormat: String, CaseIterable, Identifiable {
    case jpeg = "JPEG"
    case heic = "HEIC"
    case png  = "PNG"

    var id: String { rawValue }

    var fileExtension: String {
        switch self {
        case .jpeg: return "jpg"
        case .heic: return "heic"
        case .png:  return "png"
        }
    }

    var mimeType: String {
        switch self {
        case .jpeg: return "image/jpeg"
        case .heic: return "image/heic"
        case .png:  return "image/png"
        }
    }
}

enum ConversionError: LocalizedError {
    case unsupportedFormat
    case processingFailed
    case encodingFailed

    var errorDescription: String? {
        switch self {
        case .unsupportedFormat: return "פורמט הקובץ אינו נתמך"
        case .processingFailed:  return "עיבוד הקובץ נכשל"
        case .encodingFailed:    return "קידוד הקובץ נכשל"
        }
    }
}

struct RAWProcessor {

    private static let ciContext = CIContext(options: [.useSoftwareRenderer: false])

    static func convert(url: URL, to format: OutputFormat, quality: Float = 0.88) throws -> Data {
        if #available(iOS 15.0, *) {
            if let data = try? convertWithRAWFilter(url: url, format: format, quality: quality) {
                return data
            }
        }
        return try convertWithImageSource(url: url, format: format, quality: quality)
    }

    @available(iOS 15.0, *)
    private static func convertWithRAWFilter(url: URL, format: OutputFormat, quality: Float) throws -> Data {
        guard let filter = CIRAWFilter(imageURL: url) else {
            throw ConversionError.unsupportedFormat
        }
        filter.extendedDynamicRangeAmount = 0
        guard let ciImage = filter.outputImage else {
            throw ConversionError.processingFailed
        }
        return try encode(ciImage, format: format, quality: quality)
    }

    private static func convertWithImageSource(url: URL, format: OutputFormat, quality: Float) throws -> Data {
        let sourceOptions: [CFString: Any] = [
            kCGImageSourceShouldCache: false,
            kCGImageSourceShouldAllowFloat: false
        ]
        guard let source = CGImageSourceCreateWithURL(url as CFURL, sourceOptions as CFDictionary),
              let cgImage = CGImageSourceCreateImageAtIndex(source, 0, nil) else {
            throw ConversionError.processingFailed
        }
        let ciImage = CIImage(cgImage: cgImage)
        return try encode(ciImage, format: format, quality: quality)
    }

    private static func encode(_ image: CIImage, format: OutputFormat, quality: Float) throws -> Data {
        let sRGB = CGColorSpace(name: CGColorSpace.sRGB)!
        let compressionKey = CIImageRepresentationOption(rawValue: kCGImageDestinationLossyCompressionQuality as String)
        let opts: [CIImageRepresentationOption: Any] = [compressionKey: quality]

        let data: Data?
        switch format {
        case .jpeg:
            data = ciContext.jpegRepresentation(of: image, colorSpace: sRGB, options: opts)
        case .heic:
            data = ciContext.heifRepresentation(of: image, format: .RGBA8, colorSpace: sRGB, options: opts)
        case .png:
            data = ciContext.pngRepresentation(of: image, format: .RGBA8, colorSpace: sRGB)
        }

        guard let result = data else { throw ConversionError.encodingFailed }
        return result
    }

    static func thumbnail(data: Data, maxDim: CGFloat = 200) -> UIImage? {
        guard let source = CGImageSourceCreateWithData(data as CFData, nil) else { return nil }
        let opts: [CFString: Any] = [
            kCGImageSourceThumbnailMaxPixelSize: maxDim,
            kCGImageSourceCreateThumbnailFromImageAlways: true,
            kCGImageSourceCreateThumbnailWithTransform: true
        ]
        guard let cgThumb = CGImageSourceCreateThumbnailAtIndex(source, 0, opts as CFDictionary) else { return nil }
        return UIImage(cgImage: cgThumb)
    }
}
