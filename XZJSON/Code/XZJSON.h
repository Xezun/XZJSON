//
//  XZJSON.h
//  XZJSON
//
//  Created by 徐臻 on 2024/9/28.
//

#import <Foundation/Foundation.h>
#import "XZJSONDefines.h"


NS_ASSUME_NONNULL_BEGIN

/// 支持 模型 与 JSON 互相转换的工具。
@interface XZJSON : NSObject

/// 模型化 JSON 数据。
/// - Parameters:
///   - json: NSString 或 NSData 形式的 JSON 数据，或者它们组成的数组。
///   - options: 解析JSON的选项，如果 JSON 已解析，则此参数忽略
///   - class: 模型类
+ (nullable id)decode:(nullable id)json options:(NSJSONReadingOptions)options class:(Class)aClass;

/// JSON 化实例模型。
/// - Parameters:
///   - object: 模型对象
///   - options: JSON生成选项
///   - error: 错误输出
+ (nullable NSData *)encode:(nullable id)object options:(NSJSONWritingOptions)options error:(NSError **)error;

/// 使用 dictionary 对模型实例 object 进行模型化。
/// - Parameters:
///   - object: 模型实例对象
///   - dictionary: 数据字典
+ (void)object:(id)object decodeWithDictionary:(NSDictionary *)dictionary;

/// 将模型实例 object 序列化为 dictionary 的键值。
/// - Parameters:
///   - object: 模型实例对象
///   - dictionary: 数据字典
+ (void)object:(id)object encodeIntoDictionary:(NSMutableDictionary *)dictionary;

@end


@interface XZJSON (XZExtenedJSON)

@end

NS_ASSUME_NONNULL_END
