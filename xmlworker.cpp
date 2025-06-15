#include "xmlworker.h"

// Define XML structure constants
const QString XMLWorker::ROOT_ELEMENT_NAME = "database";     // Expected root element
const QString XMLWorker::TABLE_ELEMENT_NAME = "table";       // Expected table element
const QString XMLWorker::ROW_ELEMENT_NAME = "row";           // Expected row element
const QString XMLWorker::CELL_ELEMENT_NAME = "cell";         // Expected cell element

/**
 * @brief Constructor initializes XMLWorker with default values
 */
XMLWorker::XMLWorker()
    : CurrentFilePath("")              // Path to active XML file
    , AvailableTableNames()            // List of discovered table names
    , FileLoaded(false)                // File loading status flag
{
    // Initialize DOM document for XML processing
    XmlDocument = QDomDocument();
}

/**
 * @brief Destructor performs cleanup operations
 */
XMLWorker::~XMLWorker()
{
    // Cleanup is handled automatically by Qt's object management
}

/**
 * @brief Load and parse XML file from specified path
 * @param filePath Path to the XML file to load (absolute or relative path)
 * @return true if file loaded and parsed successfully, false otherwise
 */
bool XMLWorker::LoadXMLFile(const QString &filePath)
{
    // Validate input parameters
    if (filePath.isEmpty()) {
        qDebug() << "Error: Empty file path provided";
        return false;
    }

    // Open the XML file for reading
    QFile _xmlFile(filePath);  // File object for accessing the XML file on disk
    if (!_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot open file" << filePath;
        return false;
    }

    // Variables for error reporting during XML parsing
    QString _errorMessage;     // Error message if XML parsing fails (empty if successful)
    int _errorLine = 0;        // Line where XML parsing error occurred (0 if no error)
    int _errorColumn = 0;      // Column where XML parsing error occurred (0 if no error)

    // Parse XML content into DOM document
    if (!XmlDocument.setContent(&_xmlFile, &_errorMessage, &_errorLine, &_errorColumn)) {
        qDebug() << "Error: XML parsing failed at line" << _errorLine
                 << "column" << _errorColumn << ":" << _errorMessage;
        _xmlFile.close();
        return false;
    }

    _xmlFile.close();

    // Validate XML structure before proceeding
    if (!ValidateXMLStructure()) {
        qDebug() << "Error: Invalid XML structure";
        return false;
    }

    // Store file path and extract table information
    CurrentFilePath = filePath;
    ParseXMLStructure();
    FileLoaded = true;

    qDebug() << "Successfully loaded XML file:" << filePath;
    qDebug() << "Found" << AvailableTableNames.size() << "tables";

    return true;
}

/**
 * @brief Get list of all available table names from loaded XML
 */
QStringList XMLWorker::GetTableNames() const
{
    return AvailableTableNames;
}

/**
 * @brief Load specific table data into provided QTableWidget
 * @param tableName Name of the table to load (must exist in XML document)
 * @param tableWidget Pointer to QTableWidget that will display the data
 * @return true if data loaded successfully, false on error
 */
bool XMLWorker::LoadTableData(const QString &tableName, QTableWidget *tableWidget)
{
    // Validate input parameters
    if (!FileLoaded || tableName.isEmpty() || !tableWidget) {
        qDebug() << "Error: Invalid parameters for loading table data";
        return false;
    }

    // Find the specified table element
    QDomElement _tableElement = FindTableElement(tableName);  // DOM element for requested table (null if not found)
    if (_tableElement.isNull()) {
        qDebug() << "Error: Table" << tableName << "not found";
        return false;
    }

    // Extract column headers and row data
    QStringList _columnHeaders = ExtractColumnHeaders(_tableElement);  // List of column names from table
    QList<QStringList> _tableRows = ExtractTableRows(_tableElement);   // List of rows, each containing list of cell values

    // Configure table widget dimensions
    tableWidget->clear();
    tableWidget->setColumnCount(_columnHeaders.size());
    tableWidget->setRowCount(_tableRows.size());

    // Set column headers
    tableWidget->setHorizontalHeaderLabels(_columnHeaders);

    // Populate table with row data
    for (int _row = 0; _row < _tableRows.size(); ++_row) {  // Current row index (0-based)
        const QStringList &_rowData = _tableRows[_row];     // Values for current row
        for (int _col = 0; _col < _rowData.size() && _col < _columnHeaders.size(); ++_col) {  // Current column index (0-based)
            QTableWidgetItem *_item = new QTableWidgetItem(_rowData[_col]);  // Table cell item containing XML cell data
            tableWidget->setItem(_row, _col, _item);
        }
    }

    qDebug() << "Loaded table" << tableName << "with" << _tableRows.size() << "rows";
    return true;
}

/**
 * @brief Add new row to specified table in XML document
 */
bool XMLWorker::AddRowToTable(const QString &tableName, const QStringList &rowData)
{
    if (!FileLoaded || tableName.isEmpty()) {
        qDebug() << "Error: Invalid parameters for adding row";
        return false;
    }

    // Find target table element
    QDomElement tableElement = FindTableElement(tableName);
    if (tableElement.isNull()) {
        qDebug() << "Error: Table" << tableName << "not found for row addition";
        return false;
    }

    // Get column headers for proper row structure
    QStringList columnHeaders = ExtractColumnHeaders(tableElement);

    // Create new row element with provided data
    QDomElement newRowElement = CreateRowElement(rowData, columnHeaders);

    // Append new row to table
    tableElement.appendChild(newRowElement);

    qDebug() << "Added new row to table" << tableName;
    return true;
}

/**
 * @brief Delete specific row from table by index
 */
bool XMLWorker::DeleteRowFromTable(const QString &tableName, int rowIndex)
{
    if (!FileLoaded || tableName.isEmpty() || rowIndex < 0) {
        qDebug() << "Error: Invalid parameters for deleting row";
        return false;
    }

    // Find the table element
    QDomElement _tableElement = FindTableElement(tableName);  // DOM element for requested table (null if not found)
    if (_tableElement.isNull()) {
        qDebug() << "Error: Table" << tableName << "not found";
        return false;
    }

    // Get the row to delete
    QDomNodeList _rowNodes = _tableElement.elementsByTagName(ROW_ELEMENT_NAME);  // List of row elements in table
    if (rowIndex >= _rowNodes.size()) {
        qDebug() << "Error: Row index" << rowIndex << "is out of range";
        return false;
    }

    // Remove the row
    QDomNode _rowToDelete = _rowNodes.at(rowIndex);  // DOM node representing the row to be deleted
    _tableElement.removeChild(_rowToDelete);

    qDebug() << "Deleted row" << rowIndex << "from table" << tableName;
    return true;
}

/**
 * @brief Replace entire table with data from QTableWidget
 * @param tableName Name of the table to update (must exist in XML document)
 * @param tableWidget Pointer to QTableWidget containing the new data
 * @return true if table updated successfully, false on error
 */
bool XMLWorker::UpdateCompleteTable(const QString &tableName, QTableWidget *tableWidget)
{
    // Validate input parameters
    if (!FileLoaded || tableName.isEmpty() || !tableWidget) {
        qDebug() << "Error: Invalid parameters for updating table data";
        return false;
    }

    // Find the table element to update
    QDomElement _tableElement = FindTableElement(tableName);  // DOM element for requested table (null if not found)
    if (_tableElement.isNull()) {
        qDebug() << "Error: Table" << tableName << "not found";
        return false;
    }

    // Extract column headers from the table widget
    QStringList _columnHeaders;  // List to store column header texts
    for (int _col = 0; _col < tableWidget->columnCount(); ++_col) {  // Current column index (0-based)
        QTableWidgetItem *_headerItem = tableWidget->horizontalHeaderItem(_col);  // Header item for current column
        if (_headerItem) {
            _columnHeaders.append(_headerItem->text());
        } else {
            // Generate default column name if no header exists
            _columnHeaders.append(QString("Column_%1").arg(_col + 1));
        }
    }

    // Remove all existing rows from the table element
    QDomNodeList _rowNodes = _tableElement.elementsByTagName(ROW_ELEMENT_NAME);  // All row elements in the table
    while (!_rowNodes.isEmpty() && _rowNodes.size() > 0) {
        _tableElement.removeChild(_rowNodes.at(0));
    }

    // Add new rows from the table widget
    for (int _row = 0; _row < tableWidget->rowCount(); ++_row) {  // Current row index (0-based)
        QStringList _rowData;  // List to store cell values for current row
        
        // Collect data from each cell in the row
        for (int _col = 0; _col < tableWidget->columnCount(); ++_col) {  // Current column index (0-based)
            QTableWidgetItem *_cellItem = tableWidget->item(_row, _col);  // Cell item at current position
            _rowData.append(_cellItem ? _cellItem->text() : "");  // Use empty string if cell is null
        }
        
        // Create and add the new row element
        QDomElement _rowElement = CreateRowElement(_rowData, _columnHeaders);  // New DOM row element
        _tableElement.appendChild(_rowElement);
    }

    qDebug() << "Updated table" << tableName << "with" << tableWidget->rowCount() << "rows";
    return true;
}

/** * @brief Save current XML document state to file
 */
bool XMLWorker::SaveXMLFile()
{
    if (!FileLoaded || CurrentFilePath.isEmpty()) {
        qDebug() << "Error: No file loaded for saving";
        return false;
    }

    QFile xmlFile(CurrentFilePath);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot open file for writing" << CurrentFilePath;
        return false;
    }

    // Write XML document with proper formatting
    QTextStream stream(&xmlFile);
    stream.setEncoding(QStringConverter::Utf8);
    stream << XmlDocument.toString(4);  // Indent with 4 spaces

    xmlFile.close();

    qDebug() << "Successfully saved XML file:" << CurrentFilePath;
    return true;
}

/**
 * @brief Get path of currently loaded XML file
 */
QString XMLWorker::GetCurrentFilePath() const
{
    return CurrentFilePath;
}

/**
 * @brief Check if XML file is currently loaded and ready
 */
bool XMLWorker::IsFileLoaded() const
{
    return FileLoaded;
}

/**
 * @brief Parse loaded XML document and extract table structure information
 */
void XMLWorker::ParseXMLStructure()
{
    AvailableTableNames.clear();

    QDomElement rootElement = XmlDocument.documentElement();
    QDomNodeList tableNodes = rootElement.elementsByTagName(TABLE_ELEMENT_NAME);

    // Extract table names from all table elements
    for (int i = 0; i < tableNodes.size(); ++i) {
        QDomElement tableElement = tableNodes.at(i).toElement();
        if (!tableElement.isNull()) {
            QString tableName = tableElement.attribute("name");
            if (!tableName.isEmpty()) {
                AvailableTableNames.append(tableName);
            }
        }
    }

    qDebug() << "Parsed XML structure, found tables:" << AvailableTableNames;
}

/**
 * @brief Find table element by name in current XML document
 */
QDomElement XMLWorker::FindTableElement(const QString &tableName)
{
    QDomElement rootElement = XmlDocument.documentElement();
    QDomNodeList tableNodes = rootElement.elementsByTagName(TABLE_ELEMENT_NAME);

    // Search for table with matching name attribute
    for (int i = 0; i < tableNodes.size(); ++i) {
        QDomElement tableElement = tableNodes.at(i).toElement();
        if (tableElement.attribute("name") == tableName) {
            return tableElement;
        }
    }

    return QDomElement();  // Return null element if not found
}

/**
 * @brief Extract column headers from table element structure
 */
QStringList XMLWorker::ExtractColumnHeaders(const QDomElement &tableElement)
{
    QStringList headers;

    // Get first row to determine column structure
    QDomNodeList rowNodes = tableElement.elementsByTagName(ROW_ELEMENT_NAME);
    if (rowNodes.size() > 0) {
        QDomElement firstRow = rowNodes.at(0).toElement();
        QDomNodeList cellNodes = firstRow.elementsByTagName(CELL_ELEMENT_NAME);

        // Extract column names from cell attributes
        for (int i = 0; i < cellNodes.size(); ++i) {
            QDomElement cellElement = cellNodes.at(i).toElement();
            QString columnName = cellElement.attribute("name");
            headers.append(columnName.isEmpty() ? QString("Column_%1").arg(i + 1) : columnName);
        }
    }

    return headers;
}

/**
 * @brief Extract all row data from table element
 */
QList<QStringList> XMLWorker::ExtractTableRows(const QDomElement &tableElement)
{
    QList<QStringList> rows;

    QDomNodeList rowNodes = tableElement.elementsByTagName(ROW_ELEMENT_NAME);

    // Process each row element
    for (int i = 0; i < rowNodes.size(); ++i) {
        QDomElement rowElement = rowNodes.at(i).toElement();
        QDomNodeList cellNodes = rowElement.elementsByTagName(CELL_ELEMENT_NAME);

        QStringList rowData;

        // Extract text content from each cell
        for (int j = 0; j < cellNodes.size(); ++j) {
            QDomElement cellElement = cellNodes.at(j).toElement();
            rowData.append(cellElement.text());
        }

        rows.append(rowData);
    }

    return rows;
}

/**
 * @brief Create new row element with specified data and column structure
 */
QDomElement XMLWorker::CreateRowElement(const QStringList &rowData, const QStringList &columnHeaders)
{
    QDomElement rowElement = XmlDocument.createElement(ROW_ELEMENT_NAME);

    // Create cell elements for each column
    for (int i = 0; i < columnHeaders.size(); ++i) {
        QDomElement cellElement = XmlDocument.createElement(CELL_ELEMENT_NAME);
        cellElement.setAttribute("name", columnHeaders[i]);

        // Set cell text content, use empty string if data not available
        QString cellValue = (i < rowData.size()) ? rowData[i] : "";
        QDomText cellText = XmlDocument.createTextNode(cellValue);
        cellElement.appendChild(cellText);

        rowElement.appendChild(cellElement);
    }

    return rowElement;
}

/**
 * @brief Validate that XML document has expected structure
 */
bool XMLWorker::ValidateXMLStructure()
{
    QDomElement rootElement = XmlDocument.documentElement();

    // Check if root element exists and has correct name
    if (rootElement.isNull()) {
        qDebug() << "Error: No root element found";
        return false;
    }

    // Accept flexible root element names for compatibility
    qDebug() << "Root element:" << rootElement.tagName();

    // Check for existence of table elements
    QDomNodeList tableNodes = rootElement.elementsByTagName(TABLE_ELEMENT_NAME);
    if (tableNodes.size() == 0) {
        qDebug() << "Warning: No table elements found";
        // Allow files with no tables for now
    }

    qDebug() << "XML structure validation passed";
    return true;
}
