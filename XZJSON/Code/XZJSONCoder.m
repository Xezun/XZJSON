//
//  XZJSONCoder.m
//  XZJSON
//
//  Created by 徐臻 on 2024/9/28.
//

#import "XZJSONCoder.h"
#import "NSObject+YYModel.h"

@implementation XZJSONCoder

/// 返回 JSONObject
+ (id)_encodeObject:(id)object {
    if (object == NSNull.null) {
        return object;
    }
    if ([object isKindOfClass:NSArray.class]) {
        NSArray * const array = object;
        if (array.count == 0) {
            return array;
        }
        NSMutableArray * const arrayM = [NSMutableArray arrayWithCapacity:array.count];
        for (id item in object) {
            id obj = [self _encodeObject:item];
            if (obj) {
                [arrayM addObject:obj];
            }
        }
        return arrayM;
    }
    if ([object isKindOfClass:NSDictionary.class]) {
        NSDictionary * const dict = object;
        if (dict.count == 0) {
            return dict;
        }
        NSMutableDictionary * const dictM = [NSMutableDictionary dictionaryWithCapacity:dict.count];
        [dict enumerateKeysAndObjectsUsingBlock:^(id key, id  _Nonnull obj, BOOL * _Nonnull stop) {
            key = [NSString stringWithFormat:@"%@", key];
            obj = [self _encodeObject:obj];
            if (obj != nil) {
                dictM[key] = obj;
            }
        }];
        return dictM;
    }
    return [object encodeIntoJSONDictionary];
}
+ (NSData *)encodeObject:(id)object options:(NSJSONWritingOptions)options error:(NSError *__autoreleasing  _Nullable * _Nullable)error {
    if (object == nil) {
        return nil;
    }
    
    id const JSONObject = [self _encodeObject:object];
    
    if (JSONObject == nil) {
        return nil;
    }
    
    return [NSJSONSerialization dataWithJSONObject:JSONObject options:options error:error];
}

+ (id)decodeJSON:(id)json forClass:(Class)aClass options:(NSJSONReadingOptions)options {
    if (json == nil || json == NSNull.null) {
        return nil;
    }
    // 二进制流形式的 json 数据。
    if ([json isKindOfClass:NSData.class]) {
        return [self decodeData:json forClass:aClass options:options];
    }
    // 字符串形式的 json 数据。
    if ([json isKindOfClass:NSString.class]) {
        NSData * const data = [((NSString *)json) dataUsingEncoding:NSUTF8StringEncoding];
        if (data == nil) {
            return nil;
        }
        return [self decodeData:data forClass:aClass options:options];
    }
    // 默认数组为，解析多个 json 数据
    if ([json isKindOfClass:NSArray.class]) {
        NSMutableArray *arrayM = [NSMutableArray arrayWithCapacity:((NSArray *)json).count];
        for (id object in json) {
            id const model = [self decodeJSON:object forClass:aClass options:options];
            if (model != nil) {
                [arrayM addObject:model];
            }
        }
        return arrayM;
    }
    // 其它情况视为已解析好的 json
    return [self decodeObject:json forClass:aClass];
}

+ (nullable id)decodeData:(nonnull NSData *)data forClass:(Class)aClass options:(NSJSONReadingOptions)options {
    if (data == nil) {
        return nil;
    }
    NSError *error = nil;
    id const object = [NSJSONSerialization JSONObjectWithData:data options:options error:&error];
    if (error == nil || error.code == noErr ) {
        return [self decodeObject:object forClass:aClass];
    }
    return nil;
}

+ (nullable id)decodeObject:(nonnull id)object forClass:(Class)aClass {
    if (object == NSNull.null) {
        return nil;
    }
    if ([object isKindOfClass:NSDictionary.class]) {
        return [[aClass alloc] initWithJSONDictionary:object];
    }
    if ([object isKindOfClass:NSArray.class]) {
        NSArray * const array = object;
        if (array.count == 0) {
            return array;
        }
        
        NSMutableArray * const arrayM = [NSMutableArray arrayWithCapacity:array.count];
        for (id object in array) {
            id const model = [self decodeObject:object forClass:aClass];
            if (model) {
                [arrayM addObject:model];
            }
        }
        return arrayM;
    }
    if ([object isKindOfClass:aClass]) {
        return object;
    }
    return nil;
}

+ (void)object:(id)object decodeWithDictionary:(NSDictionary *)dictionary {
    
}

+ (void)object:(id)object encodeIntoDictionary:(NSMutableDictionary *)dictionary {
    
}

@end


@implementation NSObject (XZJSON)


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-designated-initializers"
- (instancetype)initWithJSONDictionary:(NSDictionary *)dictionary {
    return self;
}
#pragma clang diagnostic pop

- (void)xz_decodeWithDictionary:(NSDictionary *)JSON {
    
}

- (void)xz_encodeIntoDictionary:(NSMutableDictionary *)dictionary {
    
}

@end


@implementation NSString (XZJSON)

- (nullable NSDictionary *)encodeIntoJSONDictionary {
    return nil;
}

@end


@implementation NSNumber (XZJSON)

- (nullable NSDictionary *)encodeIntoJSONDictionary {
    return nil;
}

@end

@implementation NSDate (XZJSON)

- (nullable NSDictionary *)encodeIntoJSONDictionary {
    return nil;
}

@end

@implementation NSURL (XZJSON)

- (nullable NSDictionary *)encodeIntoJSONDictionary {
    return nil;
}

@end

@implementation NSData (XZJSON)

- (nullable NSDictionary *)encodeIntoJSONDictionary {
    return nil;
}

@end
