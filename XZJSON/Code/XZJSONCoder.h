//
//  XZJSONCoder.h
//  XZJSON
//
//  Created by 徐臻 on 2024/9/28.
//

#import <Foundation/Foundation.h>
#import "XZJSONDefines.h"


NS_ASSUME_NONNULL_BEGIN

@interface XZJSONCoder : NSObject

/// JSON转模型。
/// - Parameters:
///   - json: JSON
///   - aClass: 模型类
///   - options: 解析JSON的选项，如果已解析，则此参数忽略
+ (nullable id)decode:(nullable id)json options:(NSJSONReadingOptions)options class:(Class)aClass;

/// 模型转JSON
/// - Parameters:
///   - object: 模型对象
///   - options: 生成JSON选项
///   - error: 错误输出
+ (nullable NSData *)encode:(nullable id)object options:(NSJSONWritingOptions)options error:(NSError **)error;

+ (void)object:(id)object decodeWithDictionary:(NSDictionary *)dictionary;
+ (void)object:(id)object encodeIntoDictionary:(nullable NSMutableDictionary *)dictionary;
@end


@interface XZJSONCoder (NSCoder)

@end

NS_ASSUME_NONNULL_END
