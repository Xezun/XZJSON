//
//  XZJSONDefines.h
//  Pods
//
//  Created by 徐臻 on 2024/9/29.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol XZJSONDecoding <NSObject>
/// 转发模型。
+ (nullable Class)forwardingClassForJSONDictionary:(NSDictionary *)JSON;

/// 数据校验。
+ (nullable NSDictionary *)canDecodeFromJSONDictionary:(NSDictionary *)JSON;

/// JSON 转 Model 初始化方法。
///
/// 实现此方法，模型可以实现数据校验、自定义模型转换的操作。
///
/// ```objc
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
/// ```
/// - Parameter JSON: JSON 数据
- (nullable instancetype)initWithJSONDictionary:(NSDictionary *)JSON;
@end

@protocol XZJSONEncoding <NSObject>
/// 模型转JSON。
/// ```objc
/// - (nullable NSDictionary *)encodeIntoJSONDictionary {
///     NSMutableDictionary *dictionary = [NSMutableDictionary dictionary];
///     [XZJSONCoder object:self encodeIntoDictionary:dictionary];
///     return dictionary;
/// }
/// ```
- (nullable NSDictionary *)encodeIntoJSONDictionary;
@end

NS_ASSUME_NONNULL_END
