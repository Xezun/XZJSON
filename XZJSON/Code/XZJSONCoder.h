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

@interface NSObject (XZJSON)
- (void)xz_decodeWithDictionary:(NSDictionary *)JSON;
- (void)xz_encodeIntoDictionary:(NSMutableDictionary *)dictionary;
@end

NS_ASSUME_NONNULL_END
