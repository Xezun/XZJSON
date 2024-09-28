//
//  XZJSONCoder.h
//  XZJSON
//
//  Created by 徐臻 on 2024/9/28.
//

#import <Foundation/Foundation.h>


NS_ASSUME_NONNULL_BEGIN

@interface XZJSONCoder : NSObject
+ (nullable id)decodeJSON:(nullable id)json forClass:(nullable Class)aClass options:(NSJSONReadingOptions)options;
+ (nullable NSData *)encodeObject:(nullable id)object options:(NSJSONWritingOptions)options error:(NSError **)error;

+ (void)object:(id)object decodeWithDictionary:(NSDictionary *)dictionary;
+ (void)object:(id)object encodeIntoDictionary:(NSMutableDictionary *)dictionary;
@end

@protocol XZJSONEncoding <NSObject>
- (nullable NSDictionary *)encodeIntoJSONDictionary;
@end

@protocol XZJSONDecoding <NSObject>
/// JSON 转 Model 初始化方法。
///
/// @discussion 实现此方法，模型可以实现数据校验、自定义模型转换的操作。
/// @code
/// - (instancetype)initWithJSONDictionary:(NSDictionary *)JSON {
///     // 初步验证 JSON 数据是否合法
///     if (![JSON[@"type"] isKindOfClass:NSNumber.class]) {
///         return nil;
///     }
///
///     // 调用指定初始化方法，完成初始化。
///     self = [self initWithBar:[JSON[@"type"] intValue]];
///     if (self == nil) {
///         return nil;
///     }
///
///     // 调用 JSON 转模型方法
///     [XZJSONCoder object:self decodeWithDictionary:JSON];
///
///     // 验证模型是否正确
///     if (self.foo == nil) {
///         return nil;
///     }
///
///     return self;
/// }
/// @endcode
/// - Parameter JSON: JSON 数据
- (nullable instancetype)initWithJSONDictionary:(NSDictionary *)JSON;
@end

@interface NSObject (XZJSON)
- (void)xz_decodeWithDictionary:(NSDictionary *)JSON;
- (void)xz_encodeIntoDictionary:(NSMutableDictionary *)dictionary;
@end

NS_ASSUME_NONNULL_END
