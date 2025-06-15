# XML Table Editor

A professional Qt QMake C++ application for editing XML tables with a focus on code quality and reusability.

## Features

- Load and parse XML files containing tabular data
- Select specific tables from the XML file using a dropdown menu
- View and edit table data in a user-friendly interface
- Toggle between different operation modes (Add, Delete, Edit)
- Save changes back to the XML file with proper formatting
- Cancel unsaved changes and revert to the last saved state

## Technical Details

- Built with Qt framework (Core, Widgets, XML modules)
- C++17 standard
- Clean separation between UI logic (MainWindow) and XML processing (XMLWorker)
- Professional coding standards with consistent naming conventions
- Comprehensive commenting throughout the codebase
- Reusable XMLWorker class for integration in other applications

## Requirements

- Qt 6.x or Qt 5.15+
- C++17 compatible compiler
- QMake build system

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/XMLTableEditor.git

# Navigate to the project directory
cd XMLTableEditor

# Build with qmake
qmake
make
```

## License

This project is available under the MIT License.

## Author

Created by [Your Name]
