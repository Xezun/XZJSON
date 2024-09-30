//
//  XZJSONDefines.h
//  Pods
//
//  Created by 徐臻 on 2024/9/29.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#if defined(__has_attribute)
#  if __has_attribute(objc_direct) && __has_attribute(objc_direct_members)
#    define XZ_JSON_STATIC_CLASS __attribute__((objc_direct_members))
#    define XZ_JSON_STATIC_METHOD __attribute__((objc_direct))
#  endif
#else
#  define XZ_JSON_STATIC_CLASS
#  define XZ_JSON_STATIC_METHOD
#endif

/// XZJSON 功能协议
@protocol XZJSONCoding <NSObject>

@optional

/// 模型与 JOSN 的键值映射字典。
///
/// If the key in JSON/Dictionary does not match to the model's property name, implements this method and returns the additional mapper.
///
/// @code
/// {
///     "n": "Harry Pottery",
///     "p": 256,
///     "ext": { "desc": "A book written by J.K.Rowling." },
///     "ID": 100010
/// }
/// @endcode
///
/// @code
/// @interface YYBook : NSObject
/// @property NSString *name;
/// @property NSInteger page;
/// @property NSString *desc;
/// @property NSString *bookID;
/// @end
///
/// @implementation YYBook
/// + (NSDictionary *)modelCustomPropertyMapper {
///     return @{
///         @"name"  : @"n",
///         @"page"  : @"p",
///         @"desc"  : @"ext.desc",
///         @"bookID": @[@"id", @"ID", @"book_id"]
///     };
/// }
/// @end
/// @endcode
@property (class, readonly, nullable) NSDictionary<NSString *, id> *mappingJSONCodingKeys;
// `+ (nullable NSDictionary<NSString *, id> *)modelCustomPropertyMapper;`

/// 模型值类型映射，用于集合属性中的元素。
///
/// The generic class mapper for container properties.
///
/// If the property is a container object, such as NSArray/NSSet/NSDictionary,
/// implements this method and returns a property->class mapper, tells which kind of
/// object will be add to the array/set/dictionary.
///
/// @code
/// @class YYShadow, YYBorder, YYAttachment;
///
/// @interface YYAttributes
/// @property NSString *name;
/// @property NSArray *shadows;
/// @property NSSet *borders;
/// @property NSDictionary *attachments;
/// @end
///
/// @implementation YYAttributes
/// + (NSDictionary *)modelContainerPropertyGenericClass {
///     return @{
///         @"shadows" : [YYShadow class],
///         @"borders" : YYBorder.class,
///         @"attachments" : @"YYAttachment"
///     };
/// }
/// @end
/// @endcode
/// @return A class mapper.
@property (class, readonly, nullable) NSDictionary<NSString *, id> *mappingJSONCodingClasses;
//`+ (nullable NSDictionary<NSString *, id> *)modelContainerPropertyGenericClass;`

/// 不可模型化或序列化的属性。
///
/// All the properties in blacklist will be ignored in model transform process.
/// Returns nil to ignore this feature.
///
/// @return An array of property's name.
@property (class, readonly, nullable) NSArray<NSString *> *blockedJSONCodingKeys;
//+ (nullable NSArray<NSString *> *)modelPropertyBlacklist;

/// 需要进行模型化或序列化的属性。
///
/// If a property is not in the whitelist, it will be ignored in model transform process.
/// Returns nil to ignore this feature.
///
/// @return An array of property's name.
@property (class, readonly, nullable) NSArray<NSString *> *allowedJSONCodingKeys;
//+ (nullable NSArray<NSString *> *)modelPropertyWhitelist;

@end

@protocol XZJSONDecoding <XZJSONCoding>
@optional
/// 转发数据到其它模型。
/// - Parameter JSON: 字符串或二进制形式的原始 JSON 数据，或已序列化的字典或数组数据
+ (nullable Class)forwardingClassForJSONDictionary:(NSDictionary *)JSON;

/// 模型化之前的数据校验，一般为初步校验，比如校验统一格式等。
/// - Parameter JSON: 字符串或二进制形式的原始 JSON 数据，或已序列化的字典或数组数据。
/// - Returns: 返回 nil 表示无效数据，不进行模型化。
+ (nullable NSDictionary *)canDecodeFromJSONDictionary:(NSDictionary *)JSON;

/// JSON 数据模型初始化方法。如果需要自定义模型化过程，或者模型校验，可实现此方法。
/// ```objc
/// - (instancetype)initWithJSONDictionary:(NSDictionary *)JSON {
///     // 验证 JSON 数据是否合法
///     if (![JSON[@"type"] isKindOfClass:NSNumber.class]) {
///         return nil;
///     }
///
///     // 自定义了
///     // 调用自定义的指定初始化方法，完成初始化。
///     self = [self initWithBar:[JSON[@"type"] intValue]];
///     if (self == nil) {
///         return nil;
///     }
///
///     // 可以在 XZJSON 模型化的基础上，进行自定义模型化的过程，以减少代码量。
///
///     // 使用 XZJOSN 进行模型化。
///     [XZJSON object:self decodeWithDictionary:JSON];
///
///     // 验证模型是否正确
///     if (self.foo == nil) {
///         return nil;
///     }
///
///     return self;
/// }
/// ```
/// @note 如果不实现此方法，则使用 `init` 方法初始化模型对象，因此自定义了指定初始化方法的模型对象，需要实现此方法，否则指定初始化方法不会被调用。
/// - Parameter JSON: 字符串或二进制形式的原始 JSON 数据，或已序列化的字典或数组数据
- (nullable instancetype)initWithJSONDictionary:(NSDictionary *)JSON;
@end

@protocol XZJSONEncoding <XZJSONCoding>
@optional
/// 自定义或校验模型实例序列化为数据字典。
/// ```objc
/// - (nullable NSDictionary *)encodeIntoJSONDictionary:(NSMutableDictionary *)dictionary {
///     [XZJSON object:self encodeIntoDictionary:dictionary];
///     dictionary[@"date"] = @(NSDate.date.timeIntervalSince1970); // 自定义：向序列化数据中，加入一个时间戳
///     return dictionary;
/// }
/// ```
/// @note 如果需要校验 XZJSON 序列化的结果，也可以通过此方法实现。
/// - Parameter dictionary: 数据字典
- (nullable NSMutableDictionary *)encodeIntoJSONDictionary:(NSMutableDictionary *)dictionary;
@end

NS_ASSUME_NONNULL_END
