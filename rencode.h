#ifndef RENCODE_H
#define RENCODE_H

#include <QObject>
#include <QVariant>

class REncode : public QObject
{
    Q_OBJECT

    static const quint8 DEFAULT_FLOAT_BITS = 32; // Default number of bits for serialized floats, either 32 or 64 (also a parameter for dumps()).
    static const quint8 MAX_INT_LENGTH     = 64; // Maximum length of integer when written as base 10 string.
    // The bencode 'typecodes' such as i, d, etc have been extended and relocated on the base-256 character set.
    static const quint8 CHR_LIST    = 59;
    static const quint8 CHR_DICT    = 60;
    static const quint8 CHR_INT     = 61;
    static const quint8 CHR_INT1    = 62;
    static const quint8 CHR_INT2    = 63;
    static const quint8 CHR_INT4    = 64;
    static const quint8 CHR_INT8    = 65;
    static const quint8 CHR_FLOAT32 = 66;
    static const quint8 CHR_FLOAT64 = 44;
    static const quint8 CHR_TRUE    = 67;
    static const quint8 CHR_FALSE   = 68;
    static const quint8 CHR_NONE    = 69;
    static const quint8 CHR_TERM    = 127;
    // Positive integers with value embedded in typecode.
    static const quint8 INT_POS_FIXED_START = 0;
    static const quint8 INT_POS_FIXED_COUNT = 44;
    // Dictionaries with length embedded in typecode.
    static const quint8 DICT_FIXED_START = 102;
    static const quint8 DICT_FIXED_COUNT = 25;
    // Negative integers with value embedded in typecode.
    static const quint8 INT_NEG_FIXED_START = 70;
    static const quint8 INT_NEG_FIXED_COUNT = 32;
    // Strings with length embedded in typecode.
    static const quint8 STR_FIXED_START = 128;
    static const quint8 STR_FIXED_COUNT = 64;
    // Lists with length embedded in typecode.
    static const quint8 LIST_FIXED_START = STR_FIXED_START + STR_FIXED_COUNT;
    static const quint8 LIST_FIXED_COUNT = 64;

public:
    explicit REncode(QObject *parent = nullptr);

    static bool encode(QByteArray *out, const QVariant &v);
    static QVariant decode(QByteArray *in);

private:
    static bool encodeBool(QByteArray *out, bool b);
    static bool encodeInt(QByteArray *out, qint64 x);
    static bool encodeInt8(QByteArray *out, qint8 x);
    static bool encodeInt16(QByteArray *out, qint16 x);
    static bool encodeInt32(QByteArray *out, qint32 x);
    static bool encodeInt64(QByteArray *out, qint64 x);
    static bool encodeBigNum(QByteArray *out, const QString &num);
    static bool encodeNone(QByteArray *out);
    static bool encodeFloat(QByteArray *out, double x);
    static bool encodeFloat32(QByteArray *out, float x);
    static bool encodeFloat64(QByteArray *out, double x);
    static bool encodeBytes(QByteArray *out, const QByteArray &in);

    static bool decodeBool(bool *out, QByteArray *in);
    static bool decodeInt(qint64 *out, QByteArray *in);
    static bool decodeInt8(qint8 *out, QByteArray *in);
    static bool decodeInt16(qint16 *out, QByteArray *in);
    static bool decodeInt32(qint32 *out, QByteArray *in);
    static bool decodeInt64(qint64 *out, QByteArray *in);
    static bool decodeBigNum(QByteArray *out, QByteArray *in);
    static bool decodeNone(QVariant *out, QByteArray *in);
    static bool decodeFloat(double *out, QByteArray *in);
    static bool decodeFloat32(float *out, QByteArray *in);
    static bool decodeFloat64(double *out, QByteArray *in);
    static bool decodeBytes(QByteArray *out, QByteArray *in);

};

#endif // RENCODE_H
