#ifndef SHAPEMETADATA_H
#define SHAPEMETADATA_H

#include <QMap>
#include <QVariant>
#include <QString>
#include <QMetaType>
#include <QAtomicInt>

/**
 * @brief 属性值类型枚举。
 *
 * 用于 PropField 中明确声明属性值的类型，
 * PropertyPanel 据此选择对应的编辑控件。
 */
enum class PropType
{
    Text,       ///< 文本类型，使用 QLineEdit
    Number,     ///< 浮点数值类型，使用 QDoubleSpinBox
    Int,        ///< 整数数值类型，使用 QSpinBox
    Bool        ///< 布尔类型，使用 QCheckBox
};

/**
 * @brief 属性字段描述类。
 *
 * 用于 data(ShapeMeta::Props) 属性表中的每个字段，
 * 封装属性值、类型、显示名、可见性和可编辑性。
 * 提供类型安全的 get/set 方法，PropertyPanel 通过遍历属性表动态生成 UI。
 */
class PropField
{
public:
    /**
     * @brief 默认构造函数。
     */
    PropField() = default;

    /**
     * @brief 构造函数。
     *
     * @param v 属性值。
     * @param name 显示名。
     * @param type 值类型，默认根据 QVariant 自动推断。
     * @param vis 是否可见，默认 true。
     * @param edit 是否可编辑，默认 false。
     */
    PropField(const QVariant &v, const QString &name,
              PropType type = PropType::Text, bool vis = true, bool edit = false)
        : m_value(v), m_displayName(name), m_type(type), m_visible(vis), m_editable(edit)
    {}

    // ====== 值访问 ======

    /**
     * @brief 获取属性值（QVariant 形式）。
     *
     * @return 属性值。
     */
    QVariant value() const { return m_value; }

    /**
     * @brief 设置属性值。
     *
     * @param v 新的属性值。
     */
    void setValue(const QVariant &v) { m_value = v; }

    /**
     * @brief 获取属性值作为字符串。
     *
     * @return 字符串形式的属性值。
     */
    QString toString() const { return m_value.toString(); }

    /**
     * @brief 获取属性值作为双精度浮点数。
     *
     * @return 数值形式的属性值，无法转换时返回 0.0。
     */
    double toDouble() const { return m_value.toDouble(); }

    /**
     * @brief 获取属性值作为整数。
     *
     * @return 整数形式的属性值，无法转换时返回 0。
     */
    int toInt() const { return m_value.toInt(); }

    /**
     * @brief 获取属性值作为布尔值。
     *
     * @return 布尔形式的属性值。
     */
    bool toBool() const { return m_value.toBool(); }

    // ====== 显示名 ======

    /**
     * @brief 获取属性显示名。
     *
     * @return 显示名，如"宽度:"、"序列号:"。
     */
    QString displayName() const { return m_displayName; }

    /**
     * @brief 设置属性显示名。
     *
     * @param name 新的显示名。
     */
    void setDisplayName(const QString &name) { m_displayName = name; }

    // ====== 类型 ======

    /**
     * @brief 获取属性值类型。
     *
     * @return 值类型枚举。
     */
    PropType propType() const { return m_type; }

    /**
     * @brief 设置属性值类型。
     *
     * @param type 新的值类型。
     */
    void setPropType(PropType type) { m_type = type; }

    // ====== 可见性 ======

    /**
     * @brief 获取属性是否在面板中显示。
     *
     * @return true 显示；false 隐藏。
     */
    bool isVisible() const { return m_visible; }

    /**
     * @brief 设置属性是否在面板中显示。
     *
     * @param vis true 显示；false 隐藏。
     */
    void setVisible(bool vis) { m_visible = vis; }

    // ====== 可编辑性 ======

    /**
     * @brief 获取属性是否可编辑。
     *
     * @return true 可编辑；false 只读。
     */
    bool isEditable() const { return m_editable; }

    /**
     * @brief 设置属性是否可编辑。
     *
     * @param edit true 可编辑；false 只读。
     */
    void setEditable(bool edit) { m_editable = edit; }

private:
    /**
     * @brief 属性值。
     */
    QVariant m_value;

    /**
     * @brief 属性显示名。
     */
    QString m_displayName;

    /**
     * @brief 值类型，决定 PropertyPanel 使用何种编辑控件。
     */
    PropType m_type = PropType::Text;

    /**
     * @brief 是否在属性面板中显示。
     */
    bool m_visible = true;

    /**
     * @brief 是否可在属性面板中编辑。
     */
    bool m_editable = false;
};

Q_DECLARE_METATYPE(PropField)

/**
 * @brief 属性表类型别名。
 *
 * 键为自由字符串，各模块自行定义专属键名；
 * 值为 PropField，包含属性值、显示名、可见性和可编辑性。
 * PropertyPanel 通过遍历此表动态生成 UI。
 */
using PropMap = QMap<QString, PropField>;

Q_DECLARE_METATYPE(PropMap)

/**
 * @brief 图元元数据键定义。
 *
 * 统一所有图元的 data() 键值规范，
 * 避免各模块硬编码字符串和魔法数字。
 */
namespace ShapeMeta
{
    /**
     * @brief data() 键索引常量。
     */
    enum DataKey
    {
        Category = 0,   ///< 分类标识（QString），如 "DrawShape"、"CutArea"、"StoneSlab"
        ShapeType = 1,  ///< 形状类型（int），对应 ShapeMeta::Type 枚举值
        Name = 2,       ///< 名称（QString），图元的显示名称，需保证唯一
        Props = 3,      ///< 属性表（PropMap），图元自描述属性
        Id = 4          ///< 唯一ID（int），图元创建时自动分配，不可修改
    };

    /**
     * @brief 生成全局自增的唯一图元 ID。
     *
     * 使用静态 QAtomicInt 计数器，线程安全。
     * ID 从 1 开始递增，0 表示未分配。
     *
     * @return 新分配的唯一 ID。
     */
    static int nextId() {
        static QAtomicInt counter = 0;
        return counter.fetchAndAddRelaxed(1) + 1;
    }

    /**
     * @brief 形状类型枚举。
     *
     * 用于 data(ShapeMeta::ShapeType) 的值，
     * PropertyPanel 据此判断图元的几何类型以决定如何处理坐标偏移。
     */
    enum Type
    {
        Unknown = 0,    ///< 未知类型
        Rect = 1,       ///< 矩形（含 LabeledRectItem、QGraphicsRectItem）
        Line = 2,       ///< 线条（含 LabeledLineItem、QGraphicsLineItem）
        Pixmap = 3      ///< 图片（含 StoneSlabItem、QGraphicsPixmapItem）
    };
}

#endif // SHAPEMETADATA_H
