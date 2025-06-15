#ifndef XMLWORKER_H
#define XMLWORKER_H

#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QTextStream>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMap>
#include <QDebug>

/**
 * @brief Worker class for XML file operations
 * Handles all XML parsing, table manipulation, and file I/O operations
 */
class XMLWorker
{
public:
    /**
     * @brief Constructor for XMLWorker
     */
    XMLWorker();

    /**
     * @brief Destructor for XMLWorker
     */
    ~XMLWorker();

    /**
     * @brief Load XML file and parse its structure
     * @param filePath Path to the XML file to load
     * @return true if file loaded successfully, false otherwise
     */
    bool LoadXMLFile(const QString &filePath);

    /**
     * @brief Get list of available table names from loaded XML
     * @return QStringList containing all table names
     */
    QStringList GetTableNames() const;

    /**
     * @brief Load specific table data into QTableWidget
     * @param tableName Name of the table to load
     * @param tableWidget Target QTableWidget to populate
     * @return true if table loaded successfully, false otherwise
     */
    bool LoadTableData(const QString &tableName, QTableWidget *tableWidget);

    /**
     * @brief Add new row to specified table
     * @param tableName Name of the table to modify
     * @param rowData QStringList containing cell values for new row
     * @return true if row added successfully, false otherwise
     */
    bool AddRowToTable(const QString &tableName, const QStringList &rowData);

    /**
     * @brief Delete specific row from table
     * @param tableName Name of the table to modify
     * @param rowIndex Index of the row to delete (0-based)
     * @return true if row deleted successfully, false otherwise
     */
    bool DeleteRowFromTable(const QString &tableName, int rowIndex);

    /**
     * @brief Update entire table with new data
     * @param tableName Name of the table to replace
     * @param tableWidget Source QTableWidget containing new data
     * @return true if table updated successfully, false otherwise
     */
    bool UpdateCompleteTable(const QString &tableName, QTableWidget *tableWidget);

    /**
     * @brief Save all changes back to the XML file
     * @return true if file saved successfully, false otherwise
     */
    bool SaveXMLFile();

    /**
     * @brief Get current loaded file path
     * @return QString containing the file path
     */
    QString GetCurrentFilePath() const;

    /**
     * @brief Check if XML file is currently loaded
     * @return true if file is loaded, false otherwise
     */
    bool IsFileLoaded() const;

private:
    /**
     * @brief Parse XML document and extract table structure
     */
    void ParseXMLStructure();

    /**
     * @brief Find table element by name in DOM document
     * @param tableName Name of the table to find
     * @return QDomElement representing the table, null if not found
     */
    QDomElement FindTableElement(const QString &tableName);

    /**
     * @brief Extract column headers from table element
     * @param tableElement DOM element representing the table
     * @return QStringList containing column names
     */
    QStringList ExtractColumnHeaders(const QDomElement &tableElement);

    /**
     * @brief Extract row data from table element
     * @param tableElement DOM element representing the table
     * @return QList<QStringList> containing all row data
     */
    QMap<int, QStringList> ExtractTableRows(const QDomElement &tableElement);

    /**
     * @brief Create new row element with specified data
     * @param rowData QStringList containing cell values
     * @param columnHeaders QStringList containing column names
     * @return QDomElement representing the new row
     */
    QDomElement CreateRowElement(const QStringList &rowData, const QStringList &columnHeaders);

    /**
     * @brief Validate XML file structure
     * @return true if structure is valid, false otherwise
     */
    bool ValidateXMLStructure();

    // Global variables for XML operations
    QString CurrentFilePath;             // Path to currently loaded XML file (empty if no file loaded)
    QDomDocument XmlDocument;            // DOM representation of XML file (empty if no file loaded)
    QStringList AvailableTableNames;     // List of table names in current file (empty if no tables found)
    bool FileLoaded;                     // Flag indicating if file is loaded (true) or not loaded (false)

    // XML structure constants
    static const QString ROOT_ELEMENT_NAME;     // Expected root element name
    static const QString TABLE_ELEMENT_NAME;    // Expected table element name
    static const QString ROW_ELEMENT_NAME;      // Expected row element name
    static const QString CELL_ELEMENT_NAME;     // Expected cell element name
};

#endif // XMLWORKER_H
