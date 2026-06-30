import SwiftUI

struct ContentView: View {
    @StateObject private var vm = ConversionViewModel()

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                settingsBar
                Divider()

                if vm.selectedURLs.isEmpty && vm.convertedFiles.isEmpty {
                    emptyState
                } else {
                    fileListView
                }

                Divider()
                actionBar
            }
            .navigationTitle("ממיר RAW")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .navigationBarLeading) {
                    if !vm.selectedURLs.isEmpty || !vm.convertedFiles.isEmpty {
                        Button("נקה", role: .destructive) { vm.clearAll() }
                    }
                }
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button {
                        vm.showingDocumentPicker = true
                    } label: {
                        Image(systemName: "plus")
                    }
                    .disabled(vm.isConverting)
                }
            }
            .sheet(isPresented: $vm.showingDocumentPicker) {
                DocumentPicker { urls in vm.addFiles(urls) }
            }
            .sheet(isPresented: $vm.showingShareSheet) {
                ShareSheet(items: vm.shareItems)
            }
            .alert("שגיאה", isPresented: Binding(
                get: { vm.errorMessage != nil },
                set: { if !$0 { vm.errorMessage = nil } }
            )) {
                Button("אישור") { vm.errorMessage = nil }
            } message: {
                Text(vm.errorMessage ?? "")
            }
            .alert("נשמר בהצלחה!", isPresented: $vm.savedToPhotos) {
                Button("אישור") {}
            } message: {
                Text("\(vm.convertedFiles.count) תמונות נשמרו לגלריה")
            }
        }
        .environment(\.layoutDirection, .rightToLeft)
    }

    // MARK: - Settings Bar

    private var settingsBar: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                Text("פורמט יעד")
                    .font(.subheadline)
                    .foregroundStyle(.secondary)
                Spacer()
                Picker("", selection: $vm.outputFormat) {
                    ForEach(OutputFormat.allCases) { f in
                        Text(f.rawValue).tag(f)
                    }
                }
                .pickerStyle(.segmented)
                .frame(width: 200)
            }

            if vm.outputFormat != .png {
                HStack {
                    Text("איכות: \(Int(vm.quality * 100))%")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)
                    Slider(value: $vm.quality, in: 0.5...1.0, step: 0.01)
                }
            }
        }
        .padding(.horizontal)
        .padding(.vertical, 10)
        .background(Color(.systemGroupedBackground))
    }

    // MARK: - Empty State

    private var emptyState: some View {
        VStack(spacing: 24) {
            Spacer()
            Image(systemName: "camera.aperture")
                .font(.system(size: 72, weight: .thin))
                .foregroundStyle(.secondary)
            VStack(spacing: 8) {
                Text("אין קבצים נבחרים")
                    .font(.title2.weight(.semibold))
                Text("בחר קבצי RAW/ARW להמרה לפורמט הנתמך על ידי האייפון")
                    .font(.subheadline)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
                Text("נתמך: ARW · RAW · CR2 · CR3 · NEF · DNG · RAF ועוד")
                    .font(.caption)
                    .foregroundStyle(Color.secondary.opacity(0.7))
                    .multilineTextAlignment(.center)
            }
            Button {
                vm.showingDocumentPicker = true
            } label: {
                Label("בחר קבצים", systemImage: "folder.badge.plus")
                    .font(.headline)
                    .padding(.horizontal, 28)
                    .padding(.vertical, 14)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.large)
            Spacer()
        }
        .padding(32)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: - File List

    private var fileListView: some View {
        List {
            if !vm.selectedURLs.isEmpty {
                Section {
                    ForEach(vm.selectedURLs, id: \.self) { url in
                        SourceFileRow(url: url)
                    }
                    .onDelete { vm.removeFile(at: $0) }
                } header: {
                    Text("קבצי מקור (\(vm.selectedURLs.count))")
                }
            }

            if !vm.convertedFiles.isEmpty {
                Section {
                    ForEach(vm.convertedFiles) { file in
                        ConvertedFileRow(file: file)
                    }
                } header: {
                    Text("קבצים שהומרו (\(vm.convertedFiles.count))")
                }
            }
        }
        .listStyle(.insetGrouped)
    }

    // MARK: - Action Bar

    private var actionBar: some View {
        VStack(spacing: 12) {
            if vm.isConverting {
                VStack(spacing: 6) {
                    ProgressView(value: vm.progress)
                        .tint(.accentColor)
                    Text("ממיר... \(Int(vm.progress * 100))%")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .padding(.horizontal)
            }

            HStack(spacing: 12) {
                if !vm.convertedFiles.isEmpty && !vm.isConverting {
                    Button {
                        Task { await vm.saveToPhotos() }
                    } label: {
                        Label("שמור לגלריה", systemImage: "photo.badge.plus")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.bordered)

                    Button {
                        vm.prepareShare()
                    } label: {
                        Label("שתף", systemImage: "square.and.arrow.up")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.bordered)
                }

                if !vm.selectedURLs.isEmpty && !vm.isConverting {
                    Button {
                        Task { await vm.convert() }
                    } label: {
                        Label(
                            "המר \(vm.selectedURLs.count) קבצים",
                            systemImage: "arrow.triangle.2.circlepath"
                        )
                        .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                }
            }
            .padding(.horizontal)
        }
        .padding(.vertical, 14)
        .background(Color(.systemBackground))
    }
}

// MARK: - Sub-views

private struct SourceFileRow: View {
    let url: URL
    var body: some View {
        HStack(spacing: 12) {
            Image(systemName: "doc.badge.gearshape")
                .font(.title3)
                .foregroundStyle(.accentColor)
            VStack(alignment: .leading, spacing: 2) {
                Text(url.lastPathComponent)
                    .font(.subheadline)
                Text(url.pathExtension.uppercased())
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Spacer()
        }
        .padding(.vertical, 2)
    }
}

private struct ConvertedFileRow: View {
    let file: ConvertedFile
    var body: some View {
        HStack(spacing: 12) {
            if let thumb = file.thumbnail {
                Image(uiImage: thumb)
                    .resizable()
                    .scaledToFill()
                    .frame(width: 48, height: 48)
                    .clipShape(RoundedRectangle(cornerRadius: 6))
            } else {
                Image(systemName: "photo")
                    .font(.title2)
                    .frame(width: 48, height: 48)
                    .background(Color(.systemGray5))
                    .clipShape(RoundedRectangle(cornerRadius: 6))
            }
            VStack(alignment: .leading, spacing: 2) {
                Text(file.fileName)
                    .font(.subheadline)
                Text(file.fileSize)
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Spacer()
            Image(systemName: "checkmark.circle.fill")
                .foregroundStyle(.green)
        }
        .padding(.vertical, 4)
    }
}

#Preview {
    ContentView()
}
