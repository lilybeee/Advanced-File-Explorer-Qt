# Advanced Multi-Threaded File Explorer (Qt/C++)

A high-performance, asynchronous desktop file navigator engineered using the **Qt framework** and **Modern C++**. This utility features real-time background search scanning and dynamic UI layout nesting, keeping the main interface completely fluid and non-blocking during intensive file-system IO operations.

## Core Features
*   **Asynchronous Multi-Threaded Engine:** Long-running folder crawlers and keyword searches are delegated to an isolated worker background thread pool (`QThread` & `QDirIterator`), eliminating UI freezes.
*   **Dynamic Document Streamer (Preview Panel):** High-speed file previews that safely extract metadata and text lines instantly on a single-click event.
*   **Synchronized Dual-Pane Navigation:** Tree-sidebar navigation seamlessly linked to an agile, scannable main list content layout controller.
*   **OS Interoperability Layer:** Seamless integration with host operating system defaults via `QDesktopServices` to open documents and launch native binaries on double-click.

## Architecture & Qt Patterns Used
*   **Signals and Slots (Cross-Thread Communication):** Emits search result payloads dynamically across thread boundaries safely without mutating state raw pointers.
*   **Data Models & Proxies (`QSortFilterProxyModel`):** Protects underlying data layers by safely translating index lookups through custom memory storage (`Qt::UserRole`).
*   **Non-Blocking Search Architecture:** Re-engineered search query paths to use isolated file-name matches to maximize directory processing efficiency.

## Getting Started

### Prerequisites
*   Qt 5.15+ or Qt 6.x
*   C++17 Compiler (GCC / Clang / MSVC)
*   CMake or QMake Build Automation

### Installation & Compilation
```bash
# Clone the repository
git clone [https://github.com/lilybeee/Advanced-File-Explorer-Qt.git](https://github.com/lilybeee/Advanced-File-Explorer-Qt.git)
cd Advanced-File-Explorer-Qt

# Build the project using CMake
mkdir build && cd build
cmake ..
make

# Run the Explorer
./FileExplorer
