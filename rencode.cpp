#include "rencode.h"
#include <QtEndian>
#include <QDebug>

REncode::REncode(QObject *parent) : QObject(parent)
{

}

bool REncode::encode(QByteArray *out, const QVariant &v)
{
    QByteArray buf;
    auto type = v.type();

    //qDebug() << v;

    if (type == QVariant::List)
    {
        qDebug("encodeList");
        auto list = v.toList();
        if (list.size() < LIST_FIXED_COUNT)
        {
            out->append(LIST_FIXED_START + static_cast<char>(list.size()));
            for (auto it = list.begin(); it != list.end(); it++)
            {
                encode(out, *it);
            }
        }
        else
        {
            out->append(CHR_LIST);
            for (auto it = list.begin(); it != list.end(); it++)
            {
                encode(out, *it);
            }
            out->append(CHR_TERM);
        }
    }

    else if (type == QVariant::Map)
    {
        qDebug("encodeMap");
        auto map = v.toMap();
        if (map.size() < DICT_FIXED_COUNT)
        {
            out->append(DICT_FIXED_START + static_cast<char>(map.size()));
            for (auto it = map.begin(); it != map.end(); it++)
            {
                encode(out, it.key());
                encode(out, it.value());
            }
        }
        else
        {
            out->append(CHR_DICT);
            for (auto it = map.begin(); it != map.end(); it++)
            {
                encode(out, it.key());
                encode(out, it.value());
            }
            out->append(CHR_TERM);
        }
    }

    else
    {
        switch (type)
        {
        case QVariant::Char:
            encodeInt8(out, v.toChar().toLatin1());
            break;
        case QVariant::Int:
            encodeInt(out, v.toInt());
            break;
        case QVariant::Double:
            encodeFloat(out, v.toDouble());
            break;
        case QVariant::String:
            encodeBytes(out, v.toByteArray());
            break;
        case QVariant::Bool:
            encodeBool(out, v.toBool());
            break;

        default:
            encodeNone(out);
            break;
        }
    }

    return false;
}

bool REncode::encodeBool(QByteArray *out, bool b)
{
    qDebug("encodeBool");
    Q_CHECK_PTR(out);
    if (b)
    {
        out->append(static_cast<char>(CHR_TRUE));
    }
    else
    {
        out->append(static_cast<char>(CHR_FALSE));
    }
    return true;
}

bool REncode::encodeInt(QByteArray *out, qint64 x)
{
    if (x >= 0 && x < INT_POS_FIXED_COUNT)
        encodeInt8(out, INT_POS_FIXED_START + static_cast<qint8>(x));
    else if (x >= -INT_NEG_FIXED_COUNT && x < 0)
        encodeInt8(out, INT_NEG_FIXED_START - 1 - static_cast<qint8>(x));
    else if (x >= -128 && x <= 127)
        encodeInt8(out, static_cast<qint8>(x));
    else if (x >= -128 && x < 127)
        encodeInt16(out, static_cast<qint16>(x));
    else if (x >= -32768 && x < 32768 )
        encodeInt32(out, static_cast<qint32>(x));
    else if (x >= -2147483648ll && x < 2147483648ll)
        encodeInt64(out, x);
    else
        return false;

    return true;
}

bool REncode::encodeInt8(QByteArray *out, qint8 x)
{
    qDebug("encodeInt8");
    Q_CHECK_PTR(out);
    if (0 <= x && x < INT_POS_FIXED_COUNT)
    {
        out->append(INT_POS_FIXED_START + x);
        return true;
    }
    if (-INT_NEG_FIXED_COUNT <= x && x < 0)
    {
        out->append(INT_NEG_FIXED_START - 1 - x);
        return true;
    }
    return false;
}

bool REncode::encodeInt16(QByteArray *out, qint16 x)
{
    qDebug("encodeInt16");
    Q_CHECK_PTR(out);

    out->append(static_cast<char>(CHR_INT2));
    char buf[2];
    qToBigEndian<qint16>(x, buf);
    out->append(buf);

    return true;
}

bool REncode::encodeInt32(QByteArray *out, qint32 x)
{
    qDebug("encodeInt32");
    Q_CHECK_PTR(out);

    out->append(static_cast<char>(CHR_INT4));
    char buf[4];
    qToBigEndian<qint32>(x, buf);
    out->append(buf);

    return true;
}

bool REncode::encodeInt64(QByteArray *out, qint64 x)
{
    qDebug("encodeInt64");
    Q_CHECK_PTR(out);

    out->append(static_cast<char>(CHR_INT8));
    char buf[8];
    qToBigEndian<qint64>(x, buf);
    out->append(buf);

    return true;
}

bool REncode::encodeBigNum(QByteArray *out, const QString &num)
{
    qDebug("encodeBigNum");
    Q_CHECK_PTR(out);

    out->append(static_cast<char>(CHR_INT));
    out->append(num);
    out->append(static_cast<char>(CHR_TERM));

    return true;
}

bool REncode::encodeNone(QByteArray *out)
{
    qDebug("encodeNone");
    Q_CHECK_PTR(out);
    out->append(static_cast<char>(CHR_NONE));
    return true;
}

bool REncode::encodeFloat(QByteArray *out, double x)
{
    return encodeFloat64(out, x);
}

bool REncode::encodeFloat32(QByteArray *out, float x)
{
    qDebug("encodeFloat32");
    Q_CHECK_PTR(out);
    char buf[4];

    out->append(static_cast<char>(CHR_FLOAT32));
    qint32 *fake_float = reinterpret_cast<qint32*>(&x);
    qToBigEndian<qint32>(*fake_float, buf);
    out->append(buf);

    return true;
}

bool REncode::encodeFloat64(QByteArray *out, double x)
{
    qDebug("encodeFloat64");
    Q_CHECK_PTR(out);
    char buf[8];

    out->append(static_cast<char>(CHR_FLOAT64));
    qint64 *fake_float = reinterpret_cast<qint64*>(&x);
    qToBigEndian<qint64>(*fake_float, buf);
    out->append(buf);

    return true;
}

bool REncode::encodeBytes(QByteArray *out, const QByteArray &in)
{
    qDebug("encodeBytes");
    Q_CHECK_PTR(out);
    if (in.size() < STR_FIXED_COUNT)
    {
        out->append(static_cast<char>(STR_FIXED_START + in.size()));
    }
    else
    {
        QString b;
        out->append(b.sprintf("%d:", in.size()).toLocal8Bit());
    }
    out->append(in);

    return true;
}

QVariant REncode::decode(QByteArray *in)
{
    QVariant out_buf;
    QByteArray dec_buf;
    const quint8 type_code = static_cast<quint8>(in->at(0));
    in->remove(0,1);

    switch (type_code)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        qDebug("Bytes 0-9");
        int end = in->indexOf(':');
        QByteArray b(in->mid(0, end));
        in->remove(0, end);
        decodeBytes(&dec_buf, &b);
        out_buf = dec_buf;
        break;
    }
    case CHR_LIST:
    {
        qDebug("List Long");
        // List with size > 64
        // Try to find where this list ends, counting levels for nested lists/dics
        QVariantList l;
        int levels = 1;
        int pos = 0;
        for (auto it = in->begin(); it != in->end(); it++)
        {
            if (*it == CHR_LIST || *it == CHR_DICT)
                levels++;
            else if
               (*it == CHR_TERM)
                levels--;
            pos++;
            if (levels == 0)
                break;
        }
        if (levels == 0)
        {
            dec_buf = in->mid(0, pos);
            in->remove(0, pos);
            l.append(decode(&dec_buf));
        }
        out_buf = l;
        break;
    }
    case CHR_DICT:
    {
        qDebug("Dict Long");
        // Dict with size > 25
        QVariantMap m;
        int levels = 1;
        int pos = 0;
        for (auto it = in->begin(); it != in->end(); it++)
        {
            if (*it == CHR_LIST || *it == CHR_DICT)
                levels++;
            else if
               (*it == CHR_TERM)
                levels--;
            pos++;
            if (levels == 0)
                break;
        }
        if (levels == 0)
        {
            dec_buf = in->mid(0, pos);
            in->remove(0, pos);
            m.insert(decode(&dec_buf).toString(), decode(&dec_buf));
        }
        out_buf = m;
        break;
    }
    case CHR_INT:
    {
        qint64 i;
        decodeInt(&i, in);
        out_buf = i;
        break;
    }
    case CHR_INT1:
    {
        qint8 i;
        decodeInt8(&i, in);
        out_buf = i;
        break;
    }
    case CHR_INT2:
    {
        qint16 i;
        decodeInt16(&i, in);
        out_buf = i;
        break;
    }
    case CHR_INT4:
    {
        qint32 i;
        decodeInt32(&i, in);
        out_buf = i;
        break;
    }
    case CHR_INT8:
    {
        qint64 i;
        decodeInt64(&i, in);
        out_buf = i;
        break;
    }
    case CHR_FLOAT32:
    {
        float i;
        decodeFloat32(&i, in);
        out_buf = i;
        break;
    }
    case CHR_FLOAT64:
    {
        double i;
        decodeFloat64(&i, in);
        out_buf = i;
        break;
    }
    case CHR_TRUE:
    case CHR_FALSE:
    {
        bool b;
        decodeBool(&b, in);
        out_buf = b;
        break;
    }
    case CHR_NONE:
    {
        decodeNone(&out_buf, in);
        break;
    }

    // Variable type codes
    default:
    {
        if (type_code < INT_POS_FIXED_START + INT_POS_FIXED_COUNT)
        {
            const int i = type_code - INT_POS_FIXED_START;
            out_buf = i;
            qDebug() << "POS INT" << i;
        }
        else if (type_code >= INT_NEG_FIXED_START && type_code < INT_NEG_FIXED_START + INT_NEG_FIXED_COUNT)
        {
            const int i = (type_code - INT_NEG_FIXED_START + 1) * -1;
            out_buf = i;
            qDebug() << "NEG INT" << i;
        }

        // List with fixed count (below/eq 64)
        else if (type_code >= LIST_FIXED_START)
        {
            QVariantList l;
            quint8 sz = type_code - LIST_FIXED_START;
            qDebug() << "List" << sz;
            for (quint8 i = 0; i < sz; i++)
            {
                l.append(decode(in));
            }
            out_buf = l;
        }

        // Dict with fixed count (below/eq 25)
        else if(type_code >= DICT_FIXED_START && type_code < DICT_FIXED_START+DICT_FIXED_COUNT)
        {
            QVariantMap m;
            quint8 sz = type_code - DICT_FIXED_START;
            qDebug() << "Dict" << sz;
            for (quint8 i = 0; i < sz; i++)
            {
                m.insert(decode(in).toString(), decode(in));
            }
            out_buf = m;
        }
        else if(type_code >= STR_FIXED_START && type_code < STR_FIXED_START+STR_FIXED_COUNT)
        {
            quint8 sz = type_code - STR_FIXED_START;
            qDebug() << "Bytes" << sz;
            QByteArray b(in->mid(0, sz));
            in->remove(0, sz);
            decodeBytes(&dec_buf, &b);
            out_buf = dec_buf;
        }
        else
        {
            qWarning("Not implemented");
        }
    }
    }

    return out_buf;
}


bool REncode::decodeBool(bool *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    if (in->at(0) == CHR_TRUE) *out = true;
    else *out = false;
    in->remove(0, 1);

    return true;
}

bool REncode::decodeInt(qint64 *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    int idx = in->indexOf(CHR_TERM);
    bool check;
    QByteArray b(in->mid(0, idx));
    *out = b.toLongLong(&check);
    if (!check)
        *out = 0;

    return check;
}

bool REncode::decodeInt8(qint8 *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    *out = in->at(0);
    in->remove(0,1);

    return true;
}

bool REncode::decodeInt16(qint16 *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    *out = qFromBigEndian<qint16>(in->constData());
    in->remove(0,2);

    return true;
}

bool REncode::decodeInt32(qint32 *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    *out = qFromBigEndian<qint32>(in->constData());
    in->remove(0,4);

    return true;
}

bool REncode::decodeInt64(qint64 *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    *out = qFromBigEndian<qint64>(in->constData());
    in->remove(0,8);

    return true;
}

bool REncode::decodeBigNum(QByteArray *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    const int idx = in->indexOf(CHR_TERM);
    *out = in->mid(0, idx);
    in->remove(0, idx);

    return true;
}

bool REncode::decodeNone(QVariant *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    *out = QVariant(); // Null
    return true;
}

bool REncode::decodeFloat(double *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    decodeFloat64(out, in);
    return true;
}

bool REncode::decodeFloat32(float *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    qint32 *i = reinterpret_cast<qint32*>(out);
    *i = qFromBigEndian<qint32>(in);
    return true;
}

bool REncode::decodeFloat64(double *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    qint64 *i = reinterpret_cast<qint64*>(out);
    *i = qFromBigEndian<qint64>(in);
    return true;
}

bool REncode::decodeBytes(QByteArray *out, QByteArray *in)
{
    Q_CHECK_PTR(out);
    Q_CHECK_PTR(in);
    *out = *in;
    return true;
}

