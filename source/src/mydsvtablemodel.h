#ifndef MYDSVTABLEMODEL_H
#define MYDSVTABLEMODEL_H

#include <QAbstractTableModel>

template <class T>
class MyDsvData;

class MyDsvTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MyDsvTableModel(QObject *parent = 0);
    ~MyDsvTableModel();

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex())Q_DECL_OVERRIDE;

    bool loadFromFile(const QString &fileName, bool hasHeaderLine = false, QChar delim = 0);
    bool save(const QString &fileName, QChar delim = 0);

    bool hasHeaderLine() const;
    void setHasHeaderLine(bool hasHeaderLine);

private:
    void checkString(QString &col, QList<QString> &row, const QChar &character = 0);

private:
    bool m_hasHeaderLine;
    MyDsvData<QString>* m_data;
};

#endif // MYDSVTABLEMODEL_H
