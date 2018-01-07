#include <QTextStream>
#include <QFileInfo>
#include "mydsvtablemodel.h"

template<class T>
class MyDsvData
{
public:
    MyDsvData(int row = 0, int column = 0):
        m_rowCount(row),
        m_columnCount(column)
    {
        if (rowCount() > 0 && columnCount() > 0)
        {
            for (int i = 0; i < rowCount(); ++i)
            {
                QList<T> rowData;
                for (int j = 0; j < columnCount(); ++j)
                    rowData.append(T());
                m_data.insert(i, rowData);
            }
        }
    }

    T at(int row, int column) const
    {
        if (row >= rowCount() || column >= columnCount() || column >= m_data.at(row).count())
            return T();
        return m_data[row][column];
    }

    bool setValue(int row, int column, T value)
    {
        if (column >= columnCount())
        {
            m_columnCount = column + 1;
            for (int i = 0; i < rowCount(); ++i)
            {
                QList<T> rowData = m_data.at(i);
                while (rowData.size() < columnCount()) {
                    rowData.append(T());
                }
                m_data[i] = rowData;
            }
        }
        if (row >= rowCount())
        {
            m_rowCount = row + 1;
            while (m_data.size() < rowCount())
            {
                QList<T> rowData;
                while (rowData.size() < columnCount()) {
                    rowData.append(T());
                }
                m_data.append(rowData);
            }
        }

        if (column >= m_data.at(row).count())
        {
            QList<T> rowData = m_data.at(row);
            while (rowData.size() < columnCount())
                rowData.append(T());
            m_data[row] = rowData;
        }

        m_data[row][column] = value;
        return true;
    }

    int rowCount() const
    {
        return m_rowCount;
    }

    int columnCount() const
    {
        return m_columnCount;
    }

    void clear()
    {
        m_rowCount = 0;
        m_columnCount = 0;
        m_data.clear();
    }

    void append(const QList<T> &value)
    {
        if (value.size() > columnCount())
            m_columnCount = value.size();
        m_data.append(value);
        ++m_rowCount;
    }

    void insert(int i, const QList<T> &value)
    {
        if (value.size() > columnCount())
            m_columnCount = value.size();
        m_data.insert(i, value);
        ++m_rowCount;
    }

    void removeAt(int i)
    {
        m_data.removeAt(i);
        --m_rowCount;
    }

private:
    QList<QList<T> > m_data;
    int m_rowCount;
    int m_columnCount;
};

MyDsvTableModel::MyDsvTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    m_data = new MyDsvData<QString>();
}

MyDsvTableModel::~MyDsvTableModel()
{
    delete m_data;
}

int MyDsvTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_hasHeaderLine)
        return m_data->rowCount() - 1;
    else
        return m_data->rowCount();
}

int MyDsvTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data->columnCount();
}

QVariant MyDsvTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        if (m_hasHeaderLine)
            return m_data->at(0, section);
        else
            return QString::number(section);
    }
    else
    {
        return QString::number(section + 1);
    }
}

Qt::ItemFlags MyDsvTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant MyDsvTableModel::data(const QModelIndex &index, int role) const
{         
    if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        int row = m_hasHeaderLine ? index.row() + 1 : index.row();
        return m_data->at(row, index.column());
    }
    return QVariant();
}

bool MyDsvTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        int row = m_hasHeaderLine ? index.row() + 1 : index.row();
        if (m_data->setValue(row, index.column(), value.toString()))
        {
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

bool MyDsvTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginInsertRows(QModelIndex(), row, row + count - 1);
    QList<QString> list;
    for (int c = 0; c < columnCount(parent); ++c)
        list.append("");
    if (m_hasHeaderLine)
        ++row;
    for (int r = row; r < row + count; ++r)
        m_data->insert(r, list);
    endInsertRows();
    return true;
}

bool MyDsvTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    if (m_hasHeaderLine)
        ++row;
    for (int r = 0; r < count; ++r)
        m_data->removeAt(row);
    endRemoveRows();
    return true;
}

bool MyDsvTableModel::hasHeaderLine() const
{
   return m_hasHeaderLine;
}

void MyDsvTableModel::setHasHeaderLine(bool hasHeaderLine)
{
    beginResetModel();
    m_hasHeaderLine = hasHeaderLine;
    endResetModel();
}

void MyDsvTableModel::checkString(QString &value, QList<QString> &row, const QChar &character)
{
    if(value.count("\"") % 2 == 0)
    {
        if (value.startsWith(QChar('\"')) && value.endsWith( QChar('\"') ) )
        {
            value.remove(0, 1);
            value.chop(1);
        }
        value.replace("\"\"", "\"");
        row.append(value);
        if (character == QChar('\n'))
        {
            m_data->append(row);
            row.clear();
        }
        value.clear();
    }
    else
    {
        value.append(character);
    }
}

bool MyDsvTableModel::loadFromFile(const QString &fileName, bool hasHeaderLine, QChar delim)
{
    QFile file(fileName);

    if (delim == 0)
    {
        QString ext = QFileInfo(file).completeSuffix().toLower();
        if (ext == "csv")
            delim = QChar(',');
        else if (ext == "tsv")
            delim = QChar('\t');
        else
            return false;
    }

    if (delim == QChar('"'))
        return false;

    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    m_hasHeaderLine = hasHeaderLine;

    QString value;
    QTextStream in(&file);
    QList<QString> row;

    in.setCodec("UTF-8");
    beginResetModel();
    m_data->clear();
    while (true)
    {
        QChar character;
        in >> character;
        if (in.atEnd())
        {
            if (character == delim)
            {
                checkString(value, row, character);
                checkString(value, row, QChar('\n'));
            }
            else
            {
                checkString(value, row, QChar('\n'));
            }
            break;
        }
        else if (character == delim || character == QChar('\n'))
        {
            checkString(value, row, character);
        }
        else
        {
            value.append(character);
        }
    }
    file.close();
    endResetModel();
    return true;
}

bool MyDsvTableModel::save(const QString &fileName, QChar delim)
{
    QFile file(fileName);

    if (delim == 0)
    {
        QString ext = QFileInfo(file).completeSuffix().toLower();
        if (ext == "csv")
            delim = QChar(',');
        else if (ext == "tsv")
            delim = QChar('\t');
        else
            return false;
    }

    if (delim == QChar('"'))
        return false;

    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    for (int r = 0; r < m_data->rowCount(); ++r)
    {
        for (int c = 0; c < m_data->columnCount(); ++c)
        {
            QString value = m_data->at(r, c);
            value.replace("\"", "\"\"");
            if (value.contains("\"") || value.contains(delim))
                out << "\"" << value << "\"";
            else
                out << value;
            if (c == m_data->columnCount() - 1)
                out << endl;
            else
                out << delim;
        }
    }
    file.close();
    return true;
}
