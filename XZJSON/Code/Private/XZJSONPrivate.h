//
//  XZJSONPrivate.h
//  Pods
//
//  Created by 徐臻 on 2024/9/29.
//

#import <Foundation/Foundation.h>
#import <XZJSON/XZJSONClassDescriptor.h>
#import <XZJSON/XZJSONPropertyDescriptor.h>
#import <XZJSON/XZJSONDefines.h>
@import ObjectiveC;

@interface XZJSON (XZJSONPrivate)
/// 模型化已序列化的 JSON 数据。
+ (nullable id)_decodeObject:(nonnull id)object class:(nonnull Class)aClass;
@end

#pragma mark ----------

/// Parse a number value from 'id'.
FOUNDATION_STATIC_INLINE NSNumber * _Nullable NSNumberFromJSONValue(id _Nullable value) {
    if (!value || value == (id)kCFNull) {
        return nil;
    }
    if ([value isKindOfClass:[NSNumber class]]) {
        return value;
    }
    
    static NSDictionary<NSString *, NSNumber *> *_numberStrings;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _numberStrings = @{
            @"TRUE" :   @(YES),
            @"True" :   @(YES),
            @"true" :   @(YES),
            @"FALSE" :  @(NO),
            @"False" :  @(NO),
            @"false" :  @(NO),
            @"YES" :    @(YES),
            @"Yes" :    @(YES),
            @"yes" :    @(YES),
            @"NO" :     @(NO),
            @"No" :     @(NO),
            @"no" :     @(NO),
            @"NIL" :    (id)kCFNull,
            @"Nil" :    (id)kCFNull,
            @"nil" :    (id)kCFNull,
            @"NULL" :   (id)kCFNull,
            @"Null" :   (id)kCFNull,
            @"null" :   (id)kCFNull,
            @"(NULL)" : (id)kCFNull,
            @"(Null)" : (id)kCFNull,
            @"(null)" : (id)kCFNull,
            @"<NULL>" : (id)kCFNull,
            @"<Null>" : (id)kCFNull,
            @"<null>" : (id)kCFNull
        };
    });
    
    
    if ([value isKindOfClass:[NSString class]]) {
        NSNumber * const number = _numberStrings[value];
        if (number == (id)kCFNull) {
            return nil;
        }
        if (number) {
            return number;
        }
        const char * const string = ((NSString *)value).UTF8String;
        if (string == NULL) {
            return nil;
        }
        if (strchr(string, '.')) {
            double const number = atof(string);
            if (isnan(number) || isinf(number)) {
                return nil;
            }
            return @(number);
        }
        return @(atoll(string));
    }
    return nil;
}

/// Parse string to date.
FOUNDATION_STATIC_INLINE NSDate * _Nonnull NSDateFromJSONValue(__unsafe_unretained NSString * _Nonnull string) {
    typedef NSDate* (^XZJSONDateParser)(NSString *string);
    #define kParserNum 34
    static XZJSONDateParser blocks[kParserNum + 1] = {0};
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        {
            /*
             2014-01-20  // Google
             */
            NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
            formatter.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
            formatter.dateFormat = @"yyyy-MM-dd";
            blocks[10] = ^(NSString *string) { return [formatter dateFromString:string]; };
        }
        
        {
            /*
             2014-01-20 12:24:48
             2014-01-20T12:24:48   // Google
             2014-01-20 12:24:48.000
             2014-01-20T12:24:48.000
             */
            NSDateFormatter *formatter1 = [[NSDateFormatter alloc] init];
            formatter1.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter1.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
            formatter1.dateFormat = @"yyyy-MM-dd'T'HH:mm:ss";
            
            NSDateFormatter *formatter2 = [[NSDateFormatter alloc] init];
            formatter2.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter2.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
            formatter2.dateFormat = @"yyyy-MM-dd HH:mm:ss";

            NSDateFormatter *formatter3 = [[NSDateFormatter alloc] init];
            formatter3.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter3.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
            formatter3.dateFormat = @"yyyy-MM-dd'T'HH:mm:ss.SSS";

            NSDateFormatter *formatter4 = [[NSDateFormatter alloc] init];
            formatter4.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter4.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
            formatter4.dateFormat = @"yyyy-MM-dd HH:mm:ss.SSS";
            
            blocks[19] = ^(NSString *string) {
                if ([string characterAtIndex:10] == 'T') {
                    return [formatter1 dateFromString:string];
                } else {
                    return [formatter2 dateFromString:string];
                }
            };

            blocks[23] = ^(NSString *string) {
                if ([string characterAtIndex:10] == 'T') {
                    return [formatter3 dateFromString:string];
                } else {
                    return [formatter4 dateFromString:string];
                }
            };
        }
        
        {
            /*
             2014-01-20T12:24:48Z        // Github, Apple
             2014-01-20T12:24:48+0800    // Facebook
             2014-01-20T12:24:48+12:00   // Google
             2014-01-20T12:24:48.000Z
             2014-01-20T12:24:48.000+0800
             2014-01-20T12:24:48.000+12:00
             */
            NSDateFormatter *formatter = [NSDateFormatter new];
            formatter.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter.dateFormat = @"yyyy-MM-dd'T'HH:mm:ssZ";

            NSDateFormatter *formatter2 = [NSDateFormatter new];
            formatter2.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter2.dateFormat = @"yyyy-MM-dd'T'HH:mm:ss.SSSZ";

            blocks[20] = ^(NSString *string) { return [formatter dateFromString:string]; };
            blocks[24] = ^(NSString *string) { return [formatter dateFromString:string]?: [formatter2 dateFromString:string]; };
            blocks[25] = ^(NSString *string) { return [formatter dateFromString:string]; };
            blocks[28] = ^(NSString *string) { return [formatter2 dateFromString:string]; };
            blocks[29] = ^(NSString *string) { return [formatter2 dateFromString:string]; };
        }
        
        {
            /*
             Fri Sep 04 00:12:21 +0800 2015 // Weibo, Twitter
             Fri Sep 04 00:12:21.000 +0800 2015
             */
            NSDateFormatter *formatter = [NSDateFormatter new];
            formatter.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter.dateFormat = @"EEE MMM dd HH:mm:ss Z yyyy";

            NSDateFormatter *formatter2 = [NSDateFormatter new];
            formatter2.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
            formatter2.dateFormat = @"EEE MMM dd HH:mm:ss.SSS Z yyyy";

            blocks[30] = ^(NSString *string) { return [formatter dateFromString:string]; };
            blocks[34] = ^(NSString *string) { return [formatter2 dateFromString:string]; };
        }
    });
    if (!string) return nil;
    if (string.length > kParserNum) return nil;
    XZJSONDateParser parser = blocks[string.length];
    if (!parser) return nil;
    return parser(string);
    #undef kParserNum
}

/// Get the 'NSBlock' class.
FOUNDATION_STATIC_INLINE Class _Nonnull XZJSONGetBlockClass(void) {
    static Class cls;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        void (^block)(void) = ^{};
        cls = ((NSObject *)block).class;
        while (class_getSuperclass(cls) != [NSObject class]) {
            cls = class_getSuperclass(cls);
        }
    });
    return cls; // current is "NSBlock"
}

/**
 Get number from property.
 @discussion Caller should hold strong reference to the parameters before this function returns.
 @param model Should not be nil.
 @param meta  Should not be nil, meta.isCNumber should be YES, meta.getter should not be nil.
 @return A number object, or nil if failed.
 */
FOUNDATION_STATIC_INLINE NSNumber * _Nullable XZJSONEncodeNSNumberForProperty(__unsafe_unretained id _Nonnull model, __unsafe_unretained XZJSONPropertyDescriptor * _Nonnull meta) {
    switch (meta->_type & XZObjcTypeMask) {
        case XZObjcTypeBool: {
            return @(((bool (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeInt8: {
            return @(((int8_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeUInt8: {
            return @(((uint8_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeInt16: {
            return @(((int16_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeUInt16: {
            return @(((uint16_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeInt32: {
            return @(((int32_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeUInt32: {
            return @(((uint32_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeInt64: {
            return @(((int64_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeUInt64: {
            return @(((uint64_t (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter));
        }
        case XZObjcTypeFloat: {
            float num = ((float (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter);
            if (isnan(num) || isinf(num)) return nil;
            return @(num);
        }
        case XZObjcTypeDouble: {
            double num = ((double (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter);
            if (isnan(num) || isinf(num)) return nil;
            return @(num);
        }
        case XZObjcTypeLongDouble: {
            double num = ((long double (*)(id, SEL))(void *) objc_msgSend)((id)model, meta->_getter);
            if (isnan(num) || isinf(num)) return nil;
            return @(num);
        }
        default: return nil;
    }
}

/**
 Set number to property.
 @discussion Caller should hold strong reference to the parameters before this function returns.
 @param model Should not be nil.
 @param num   Can be nil.
 @param meta  Should not be nil, meta.isCNumber should be YES, meta.setter should not be nil.
 */
FOUNDATION_STATIC_INLINE void XZJSONDecodeNSNumberForProperty(__unsafe_unretained id _Nonnull model, __unsafe_unretained NSNumber * _Nonnull num, __unsafe_unretained XZJSONPropertyDescriptor * _Nonnull meta) {
    switch (meta->_type & XZObjcTypeMask) {
        case XZObjcTypeBool: {
            ((void (*)(id, SEL, bool))(void *) objc_msgSend)((id)model, meta->_setter, num.boolValue);
            break;
        }
        case XZObjcTypeInt8: {
            ((void (*)(id, SEL, int8_t))(void *) objc_msgSend)((id)model, meta->_setter, (int8_t)num.charValue);
            break;
        }
        case XZObjcTypeUInt8: {
            ((void (*)(id, SEL, uint8_t))(void *) objc_msgSend)((id)model, meta->_setter, (uint8_t)num.unsignedCharValue);
            break;
        }
        case XZObjcTypeInt16: {
            ((void (*)(id, SEL, int16_t))(void *) objc_msgSend)((id)model, meta->_setter, (int16_t)num.shortValue);
            break;
        }
        case XZObjcTypeUInt16: {
            ((void (*)(id, SEL, uint16_t))(void *) objc_msgSend)((id)model, meta->_setter, (uint16_t)num.unsignedShortValue);
            break;
        }
        case XZObjcTypeInt32: {
            ((void (*)(id, SEL, int32_t))(void *) objc_msgSend)((id)model, meta->_setter, (int32_t)num.intValue);
        }
        case XZObjcTypeUInt32: {
            ((void (*)(id, SEL, uint32_t))(void *) objc_msgSend)((id)model, meta->_setter, (uint32_t)num.unsignedIntValue);
            break;
        }
        case XZObjcTypeInt64: {
            if ([num isKindOfClass:[NSDecimalNumber class]]) {
                ((void (*)(id, SEL, int64_t))(void *) objc_msgSend)((id)model, meta->_setter, (int64_t)num.stringValue.longLongValue);
            } else {
                ((void (*)(id, SEL, uint64_t))(void *) objc_msgSend)((id)model, meta->_setter, (uint64_t)num.longLongValue);
            }
            break;
        }
        case XZObjcTypeUInt64: {
            if ([num isKindOfClass:[NSDecimalNumber class]]) {
                ((void (*)(id, SEL, int64_t))(void *) objc_msgSend)((id)model, meta->_setter, (int64_t)num.stringValue.longLongValue);
            } else {
                ((void (*)(id, SEL, uint64_t))(void *) objc_msgSend)((id)model, meta->_setter, (uint64_t)num.unsignedLongLongValue);
            }
            break;
        }
        case XZObjcTypeFloat: {
            float f = num.floatValue;
            if (isnan(f) || isinf(f)) f = 0;
            ((void (*)(id, SEL, float))(void *) objc_msgSend)((id)model, meta->_setter, f);
            break;
        }
        case XZObjcTypeDouble: {
            double d = num.doubleValue;
            if (isnan(d) || isinf(d)) d = 0;
            ((void (*)(id, SEL, double))(void *) objc_msgSend)((id)model, meta->_setter, d);
            break;
        }
        case XZObjcTypeLongDouble: {
            long double d = num.doubleValue;
            if (isnan(d) || isinf(d)) d = 0;
            ((void (*)(id, SEL, long double))(void *) objc_msgSend)((id)model, meta->_setter, (long double)d);
            break;
        }
        default: {
            break;
        }
    }
}



/// Get the value with key paths from dictionary
/// The dic should be NSDictionary, and the keyPath should not be nil.
FOUNDATION_STATIC_INLINE id _Nullable XZJSONDecodingGetValueWithKeyPath(__unsafe_unretained NSDictionary * _Nonnull dic, __unsafe_unretained NSArray * _Nonnull keyPaths) {
    id value = nil;
    for (NSUInteger i = 0, max = keyPaths.count; i < max; i++) {
        value = dic[keyPaths[i]];
        if (i + 1 < max) {
            if ([value isKindOfClass:[NSDictionary class]]) {
                dic = value;
            } else {
                return nil;
            }
        }
    }
    return value;
}

/// Get the value with multi key (or key path) from dictionary
/// The dic should be NSDictionary
FOUNDATION_STATIC_INLINE id _Nullable XZJSONDecodingGetValueWithKeyArray(__unsafe_unretained NSDictionary * _Nonnull dic, __unsafe_unretained NSArray * _Nonnull multiKeys) {
    id value = nil;
    for (NSString *key in multiKeys) {
        if ([key isKindOfClass:[NSString class]]) {
            value = dic[key];
            if (value) break;
        } else {
            value = XZJSONDecodingGetValueWithKeyPath(dic, (NSArray *)key);
            if (value) break;
        }
    }
    return value;
}


typedef struct {
    void * _Nonnull descriptor;  ///< XZJSONObjcClassMeta
    void * _Nonnull model;      ///< id (self)
    void * _Nonnull dictionary; ///< NSDictionary (json)
} XZJSONCodingContext;

FOUNDATION_STATIC_INLINE NSString * _Nullable NSStringFromJSONValue(id _Nullable value) {
    if (!value || value == (id)kCFNull) {
        return nil;
    }
    if ([value isKindOfClass:NSString.class]) {
        return value;
    }
    if ([value isKindOfClass:NSNumber.class]) {
        NSNumber * const number = value;
        return number.stringValue;
    }
    if ([value isKindOfClass:NSData.class]) {
        return [[NSString alloc] initWithData:value encoding:NSUTF8StringEncoding];;
    }
    return nil;
}

FOUNDATION_STATIC_INLINE NSData * _Nullable NSDataFromJSONValue(id _Nullable value) {
    if ([value isKindOfClass:NSString.class]) {
        // base64 / hex string to data
        if ([value hasPrefix:@"base64:"]) {
            NSString *string = [value substringFromIndex:7];
            return [[NSData alloc] initWithBase64EncodedString:string options:(NSDataBase64DecodingIgnoreUnknownCharacters)];
        }
        return [value dataUsingEncoding:(NSUTF8StringEncoding) allowLossyConversion:YES];
    }
    if ([value isKindOfClass:NSData.class]) {
        return value;
    }
    return nil;
}

/**
 Set value to model with a property meta.
 
 @discussion Caller should hold strong reference to the parameters before this function returns.
 
 @param model Should not be nil.
 @param value Should not be nil, but can be NSNull.
 @param property  Should not be nil, and meta->_setter should not be nil.
 */
FOUNDATION_STATIC_INLINE void XZJSONDecodeValueForProperty(__unsafe_unretained id _Nonnull model, __unsafe_unretained id _Nonnull value, __unsafe_unretained XZJSONPropertyDescriptor * _Nonnull property) {
    if (property->_isCNumber) {
        NSNumber *num = NSNumberFromJSONValue(value);
        XZJSONDecodeNSNumberForProperty(model, num, property);
    } else if (property->_nsType) {
        if (value == (id)kCFNull) {
            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, (id)nil);
        } else {
            switch (property->_nsType) {
                case XZJSONEncodingNSString: {
                    NSString * const string = NSStringFromJSONValue(value);
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, string);
                    break;
                }
                case XZJSONEncodingNSMutableString: {
                    NSString * const string = NSStringFromJSONValue(value);
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, string.mutableCopy);
                    break;
                }
                case XZJSONEncodingNSValue:{ // XZJSONEncodingNSValue
                    if ([value isKindOfClass:[NSValue class]]) {
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, value);
                    }
                    break;
                }
                case XZJSONEncodingNSNumber: {
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, NSNumberFromJSONValue(value));
                    break;
                }
                case XZJSONEncodingNSDecimalNumber: {
                    if ([value isKindOfClass:[NSNumber class]]) {
                        NSDecimalNumber *number = [NSDecimalNumber decimalNumberWithDecimal:[((NSNumber *)value) decimalValue]];
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, number);
                    } else if ([value isKindOfClass:[NSString class]]) {
                        NSDecimalNumber * const number = [NSDecimalNumber decimalNumberWithString:value];
                        NSDecimal const numberValue = number.decimalValue;
                        if (numberValue._length == 0 && numberValue._isNegative) {
                            return;
                        }
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, number);
                    } else if ([value isKindOfClass:[NSDecimalNumber class]]) {
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, value);
                    }
                    break;
                }
                    
                case XZJSONEncodingNSData: {
                    NSData * const data = NSDataFromJSONValue(value);
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, data);
                    break;
                }
                case XZJSONEncodingNSMutableData: {
                    NSMutableData * const data = [NSDataFromJSONValue(value) mutableCopy];
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, data);
                    break;
                }
                
                case XZJSONEncodingNSDate: {
                    // TODO: Date decoding
                    if ([value isKindOfClass:[NSDate class]]) {
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, value);
                    } else if ([value isKindOfClass:[NSString class]]) {
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, NSDateFromJSONValue(value));
                    }
                    break;
                }
                    
                case XZJSONEncodingNSURL: {
                    if ([value isKindOfClass:[NSURL class]]) {
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, value);
                    } else if ([value isKindOfClass:[NSString class]]) {
                        NSCharacterSet *set = [NSCharacterSet whitespaceAndNewlineCharacterSet];
                        NSString *str = [value stringByTrimmingCharactersInSet:set];
                        if (str.length == 0) {
                            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, nil);
                        } else {
                            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, [[NSURL alloc] initWithString:str]);
                        }
                    }
                    break;
                }
                    
                case XZJSONEncodingNSArray:
                case XZJSONEncodingNSMutableArray: {
                    if (property->_elementClass) {
                        NSArray *valueArr = nil;
                        if ([value isKindOfClass:[NSArray class]]) valueArr = value;
                        else if ([value isKindOfClass:[NSSet class]]) valueArr = ((NSSet *)value).allObjects;
                        if (valueArr) {
                            NSMutableArray *objectArr = [NSMutableArray new];
                            for (id one in valueArr) {
                                id const model = [XZJSON _decodeObject:one class:property->_elementClass];
                                if (model) {
                                    [objectArr addObject:model];
                                }
                            }
                            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, objectArr);
                        }
                    } else {
                        if ([value isKindOfClass:[NSArray class]]) {
                            if (property->_nsType == XZJSONEncodingNSArray) {
                                ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, value);
                            } else {
                                ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model,
                                                                               property->_setter,
                                                                               ((NSArray *)value).mutableCopy);
                            }
                        } else if ([value isKindOfClass:[NSSet class]]) {
                            if (property->_nsType == XZJSONEncodingNSArray) {
                                ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, ((NSSet *)value).allObjects);
                            } else {
                                ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model,
                                                                               property->_setter,
                                                                               ((NSSet *)value).allObjects.mutableCopy);
                            }
                        }
                    }
                    break;
                }
                    
                case XZJSONEncodingNSDictionary:
                case XZJSONEncodingNSMutableDictionary: {
                    if ([value isKindOfClass:[NSDictionary class]]) {
                        if (property->_elementClass) {
                            NSMutableDictionary *dic = [NSMutableDictionary new];
                            [((NSDictionary *)value) enumerateKeysAndObjectsUsingBlock:^(NSString *oneKey, id oneValue, BOOL *stop) {
                                id const model = [XZJSON _decodeObject:oneValue class:property->_elementClass];
                                if (model) {
                                    dic[oneKey] = model;
                                }
                            }];
                            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, dic);
                        } else {
                            if (property->_nsType == XZJSONEncodingNSDictionary) {
                                ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, value);
                            } else {
                                ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model,
                                                                               property->_setter,
                                                                               ((NSDictionary *)value).mutableCopy);
                            }
                        }
                    }
                    break;
                }
                    
                case XZJSONEncodingNSSet:
                case XZJSONEncodingNSMutableSet: {
                    NSSet *valueSet = nil;
                    if ([value isKindOfClass:[NSArray class]]) valueSet = [NSMutableSet setWithArray:value];
                    else if ([value isKindOfClass:[NSSet class]]) valueSet = ((NSSet *)value);
                    
                    if (property->_elementClass) {
                        NSMutableSet *set = [NSMutableSet new];
                        for (id one in valueSet) {
                            id const model = [XZJSON _decodeObject:one class:property->_elementClass];
                            if (model) {
                                [set addObject:model];
                            }
                        }
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, set);
                    } else {
                        if (property->_nsType == XZJSONEncodingNSSet) {
                            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, valueSet);
                        } else {
                            ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model,
                                                                           property->_setter,
                                                                           ((NSSet *)valueSet).mutableCopy);
                        }
                    }
                } // break; commented for code coverage in next line
                    
                default: break;
            }
        }
    } else {
        BOOL isNull = (value == (id)kCFNull);
        switch (property->_type & XZObjcTypeMask) {
            case XZObjcTypeObject: {
                if (isNull) {
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, (id)nil);
                } else if ([value isKindOfClass:property->_class] || !property->_class) {
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, (id)value);
                } else if ([value isKindOfClass:[NSDictionary class]]) {
                    NSObject *one = nil;
                    if (property->_getter) {
                        one = ((id (*)(id, SEL))(void *) objc_msgSend)((id)model, property->_getter);
                    }
                    if (one) {
                        [XZJSON object:one decodeWithDictionary:value];
                    } else {
                        one = [XZJSON _decodeObject:value class:property->_class];
                        // if one == nil ?
                        ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)model, property->_setter, (id)one);
                    }
                }
                break;
            }
                
            case XZObjcTypeClass: {
                if (isNull) {
                    ((void (*)(id, SEL, Class))(void *) objc_msgSend)((id)model, property->_setter, (Class)NULL);
                } else {
                    Class cls = nil;
                    if ([value isKindOfClass:[NSString class]]) {
                        cls = NSClassFromString(value);
                        if (cls) {
                            ((void (*)(id, SEL, Class))(void *) objc_msgSend)((id)model, property->_setter, (Class)cls);
                        }
                    } else {
                        cls = object_getClass(value);
                        if (cls) {
                            if (class_isMetaClass(cls)) {
                                ((void (*)(id, SEL, Class))(void *) objc_msgSend)((id)model, property->_setter, (Class)value);
                            }
                        }
                    }
                }
                break;
            }
                
            case  XZObjcTypeSEL: {
                if (isNull) {
                    ((void (*)(id, SEL, SEL))(void *) objc_msgSend)((id)model, property->_setter, (SEL)NULL);
                } else if ([value isKindOfClass:[NSString class]]) {
                    SEL sel = NSSelectorFromString(value);
                    if (sel) ((void (*)(id, SEL, SEL))(void *) objc_msgSend)((id)model, property->_setter, (SEL)sel);
                }
                break;
            }
                
            case XZObjcTypeBlock: {
                if (isNull) {
                    ((void (*)(id, SEL, void (^)(void)))(void *) objc_msgSend)((id)model, property->_setter, (void (^)(void))NULL);
                } else if ([value isKindOfClass:XZJSONGetBlockClass()]) {
                    ((void (*)(id, SEL, void (^)(void)))(void *) objc_msgSend)((id)model, property->_setter, (void (^)(void))value);
                }
                break;
            }
                
            case XZObjcTypeStruct:
            case XZObjcTypeUnion:
            case XZObjcTypeCArray: {
                if ([value isKindOfClass:[NSValue class]]) {
                    const char *valueType = ((NSValue *)value).objCType;
                    const char *metaType = property->_property.typeEncoding.UTF8String;
                    if (valueType && metaType && strcmp(valueType, metaType) == 0) {
                        [model setValue:value forKey:property->_name];
                    }
                }
                break;
            }
                
            case XZObjcTypePointer:
            case XZObjcTypeCString: {
                if (isNull) {
                    ((void (*)(id, SEL, void *))(void *) objc_msgSend)((id)model, property->_setter, (void *)NULL);
                } else if ([value isKindOfClass:[NSValue class]]) {
                    NSValue *nsValue = value;
                    if (nsValue.objCType && strcmp(nsValue.objCType, "^v") == 0) {
                        ((void (*)(id, SEL, void *))(void *) objc_msgSend)((id)model, property->_setter, nsValue.pointerValue);
                    }
                }
                break;
            } 
                
            default: break;
        }
    }
}

/**
 Apply function for dictionary, to set the key-value pair to model.
 
 @param _key     should not be nil, NSString.
 @param _value   should not be nil.
 @param _context _context.modelMeta and _context.model should not be nil.
 */
FOUNDATION_STATIC_INLINE void XZJSONDecodingDictionaryEnumerator(const void * _Nonnull _key, const void * _Nonnull _value, void * _Nonnull _context) {
    XZJSONCodingContext *context = _context;
    __unsafe_unretained XZJSONClassDescriptor *meta = (__bridge XZJSONClassDescriptor *)(context->descriptor);
    __unsafe_unretained XZJSONPropertyDescriptor *propertyMeta = [meta->_keyProperties objectForKey:(__bridge id)(_key)];
    __unsafe_unretained id model = (__bridge id)(context->model);
    while (propertyMeta) {
        if (propertyMeta->_setter) {
            XZJSONDecodeValueForProperty(model, (__bridge __unsafe_unretained id)_value, propertyMeta);
        }
        propertyMeta = propertyMeta->_next;
    };
}

/**
 Apply function for model property meta, to set dictionary to model.
 
 @param _propertyMeta should not be nil, XZJSONObjcPropertyMeta.
 @param _context      _context.model and _context.dictionary should not be nil.
 */
FOUNDATION_STATIC_INLINE void XZJSONDecodingArrayEnumerator(const void * _Nonnull _propertyMeta, void * _Nonnull _context) {
    XZJSONCodingContext *context = _context;
    __unsafe_unretained NSDictionary *dictionary = (__bridge NSDictionary *)(context->dictionary);
    __unsafe_unretained XZJSONPropertyDescriptor *propertyMeta = (__bridge XZJSONPropertyDescriptor *)(_propertyMeta);
    if (!propertyMeta->_setter) return;
    id value = nil;
    
    if (propertyMeta->_JSONKeyArray) {
        value = XZJSONDecodingGetValueWithKeyArray(dictionary, propertyMeta->_JSONKeyArray);
    } else if (propertyMeta->_JSONKeyPath) {
        value = XZJSONDecodingGetValueWithKeyPath(dictionary, propertyMeta->_JSONKeyPath);
    } else {
        value = [dictionary objectForKey:propertyMeta->_JSONKey];
    }
    
    if (value) {
        __unsafe_unretained id model = (__bridge id)(context->model);
        XZJSONDecodeValueForProperty(model, value, propertyMeta);
    }
}







/**
 Get the ISO date formatter.
 
 ISO8601 format example:
 2010-07-09T16:13:30+12:00
 2011-01-11T11:11:11+0000
 2011-01-26T19:06:43Z
 
 length: 20/24/25
 */
FOUNDATION_STATIC_INLINE NSDateFormatter * _Nonnull XZJSONDateFormatter(void) {
    static NSDateFormatter *formatter = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        formatter = [[NSDateFormatter alloc] init];
        formatter.locale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
        formatter.dateFormat = @"yyyy-MM-dd'T'HH:mm:ssZ";
    });
    return formatter;
}






/**
 Returns a valid JSON object (NSArray/NSDictionary/NSString/NSNumber/NSNull),
 or nil if an error occurs.
 
 @param model Model, can be nil.
 @return JSON object, nil if an error occurs.
 */
FOUNDATION_STATIC_INLINE id _Nonnull XZJSONEncodingRecursive(NSObject * _Nonnull model) {
    if (!model || model == (id)kCFNull) return model;
    if ([model isKindOfClass:[NSString class]]) return model;
    if ([model isKindOfClass:[NSNumber class]]) return model;
    if ([model isKindOfClass:[NSDictionary class]]) {
        if ([NSJSONSerialization isValidJSONObject:model]) return model;
        NSMutableDictionary *newDic = [NSMutableDictionary new];
        [((NSDictionary *)model) enumerateKeysAndObjectsUsingBlock:^(NSString *key, id obj, BOOL *stop) {
            NSString *stringKey = [key isKindOfClass:[NSString class]] ? key : key.description;
            if (!stringKey) return;
            id jsonObj = XZJSONEncodingRecursive(obj);
            if (!jsonObj) jsonObj = (id)kCFNull;
            newDic[stringKey] = jsonObj;
        }];
        return newDic;
    }
    if ([model isKindOfClass:[NSSet class]]) {
        NSArray *array = ((NSSet *)model).allObjects;
        if ([NSJSONSerialization isValidJSONObject:array]) return array;
        NSMutableArray *newArray = [NSMutableArray new];
        for (id obj in array) {
            if ([obj isKindOfClass:[NSString class]] || [obj isKindOfClass:[NSNumber class]]) {
                [newArray addObject:obj];
            } else {
                id jsonObj = XZJSONEncodingRecursive(obj);
                if (jsonObj && jsonObj != (id)kCFNull) [newArray addObject:jsonObj];
            }
        }
        return newArray;
    }
    if ([model isKindOfClass:[NSArray class]]) {
        if ([NSJSONSerialization isValidJSONObject:model]) return model;
        NSMutableArray *newArray = [NSMutableArray new];
        for (id obj in (NSArray *)model) {
            if ([obj isKindOfClass:[NSString class]] || [obj isKindOfClass:[NSNumber class]]) {
                [newArray addObject:obj];
            } else {
                id jsonObj = XZJSONEncodingRecursive(obj);
                if (jsonObj && jsonObj != (id)kCFNull) [newArray addObject:jsonObj];
            }
        }
        return newArray;
    }
    if ([model isKindOfClass:[NSURL class]]) return ((NSURL *)model).absoluteString;
    if ([model isKindOfClass:[NSAttributedString class]]) return ((NSAttributedString *)model).string;
    if ([model isKindOfClass:[NSDate class]]) return [XZJSONDateFormatter() stringFromDate:(id)model];
    if ([model isKindOfClass:[NSData class]]) return nil;
    
    
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:[model class]];
    if (!modelMeta || modelMeta->_numberOfProperties == 0) return nil;
    NSMutableDictionary *result = [[NSMutableDictionary alloc] initWithCapacity:64];
    __unsafe_unretained NSMutableDictionary *dic = result; // avoid retain and release in block
    [modelMeta->_keyProperties enumerateKeysAndObjectsUsingBlock:^(NSString *propertyMappedKey, XZJSONPropertyDescriptor *propertyMeta, BOOL *stop) {
        if (!propertyMeta->_getter) return;
        
        id value = nil;
        if (propertyMeta->_isCNumber) {
            value = XZJSONEncodeNSNumberForProperty(model, propertyMeta);
        } else if (propertyMeta->_nsType) {
            id v = ((id (*)(id, SEL))(void *) objc_msgSend)((id)model, propertyMeta->_getter);
            value = XZJSONEncodingRecursive(v);
        } else {
            switch (propertyMeta->_type & XZObjcTypeMask) {
                case XZObjcTypeObject: {
                    id v = ((id (*)(id, SEL))(void *) objc_msgSend)((id)model, propertyMeta->_getter);
                    value = XZJSONEncodingRecursive(v);
                    if (value == (id)kCFNull) value = nil;
                    break;
                }
                case XZObjcTypeClass: {
                    Class v = ((Class (*)(id, SEL))(void *) objc_msgSend)((id)model, propertyMeta->_getter);
                    value = v ? NSStringFromClass(v) : nil;
                    break;
                }
                case XZObjcTypeSEL: {
                    SEL v = ((SEL (*)(id, SEL))(void *) objc_msgSend)((id)model, propertyMeta->_getter);
                    value = v ? NSStringFromSelector(v) : nil;
                    break;
                }
                default: break;
            }
        }
        if (!value) return;
        
        if (propertyMeta->_JSONKeyPath) {
            NSMutableDictionary *superDic = dic;
            NSMutableDictionary *subDic = nil;
            for (NSUInteger i = 0, max = propertyMeta->_JSONKeyPath.count; i < max; i++) {
                NSString *key = propertyMeta->_JSONKeyPath[i];
                if (i + 1 == max) { // end
                    if (!superDic[key]) superDic[key] = value;
                    break;
                }
                
                subDic = superDic[key];
                if (subDic) {
                    if ([subDic isKindOfClass:[NSDictionary class]]) {
                        subDic = subDic.mutableCopy;
                        superDic[key] = subDic;
                    } else {
                        break;
                    }
                } else {
                    subDic = [NSMutableDictionary new];
                    superDic[key] = subDic;
                }
                superDic = subDic;
                subDic = nil;
            }
        } else {
            if (!dic[propertyMeta->_JSONKey]) {
                dic[propertyMeta->_JSONKey] = value;
            }
        }
    }];
    
//    if (modelMeta->_hasCustomTransformToDictionary) {
//        BOOL suc = [((id<YYModel>)model) modelCustomTransformToDictionary:dic];
//        if (!suc) return nil;
//    }
    return result;
}

/// Add indent to string (exclude first line)
FOUNDATION_STATIC_INLINE NSMutableString * _Nonnull XZJSONDescriptionWithIndent(NSMutableString * _Nonnull desc, NSUInteger indent) {
    for (NSUInteger i = 0, max = desc.length; i < max; i++) {
        unichar c = [desc characterAtIndex:i];
        if (c == '\n') {
            for (NSUInteger j = 0; j < indent; j++) {
                [desc insertString:@"    " atIndex:i + 1];
            }
            i += indent * 4;
            max += indent * 4;
        }
    }
    return desc;
}

/// Generaate a description string
FOUNDATION_STATIC_INLINE NSString * _Nonnull XZJSONDescription(NSObject * _Nonnull model) {
    static const int kDescMaxLength = 100;
    if (!model) return @"<nil>";
    if (model == (id)kCFNull) return @"<null>";
    if (![model isKindOfClass:[NSObject class]]) return [NSString stringWithFormat:@"%@",model];
    
    
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:model.class];
    switch (modelMeta->_nsType) {
        case XZJSONEncodingNSString: case XZJSONEncodingNSMutableString: {
            return [NSString stringWithFormat:@"\"%@\"",model];
        }
        
        case XZJSONEncodingNSValue:
        case XZJSONEncodingNSData: case XZJSONEncodingNSMutableData: {
            NSString *tmp = model.description;
            if (tmp.length > kDescMaxLength) {
                tmp = [tmp substringToIndex:kDescMaxLength];
                tmp = [tmp stringByAppendingString:@"..."];
            }
            return tmp;
        }
            
        case XZJSONEncodingNSNumber:
        case XZJSONEncodingNSDecimalNumber:
        case XZJSONEncodingNSDate:
        case XZJSONEncodingNSURL: {
            return [NSString stringWithFormat:@"%@",model];
        }
            
        case XZJSONEncodingNSSet: case XZJSONEncodingNSMutableSet: {
            model = ((NSSet *)model).allObjects;
        } // no break
            
        case XZJSONEncodingNSArray: case XZJSONEncodingNSMutableArray: {
            NSArray *array = (id)model;
            NSMutableString *desc = [NSMutableString new];
            if (array.count == 0) {
                return [desc stringByAppendingString:@"[]"];
            } else {
                [desc appendFormat:@"[\n"];
                for (NSUInteger i = 0, max = array.count; i < max; i++) {
                    NSObject *obj = array[i];
                    [desc appendString:@"    "];
                    [desc appendString:XZJSONDescriptionWithIndent(XZJSONDescription(obj).mutableCopy, 1)];
                    [desc appendString:(i + 1 == max) ? @"\n" : @";\n"];
                }
                [desc appendString:@"]"];
                return desc;
            }
        }
        case XZJSONEncodingNSDictionary: case XZJSONEncodingNSMutableDictionary: {
            NSDictionary *dic = (id)model;
            NSMutableString *desc = [NSMutableString new];
            if (dic.count == 0) {
                return [desc stringByAppendingString:@"{}"];
            } else {
                NSArray *keys = dic.allKeys;
                
                [desc appendFormat:@"{\n"];
                for (NSUInteger i = 0, max = keys.count; i < max; i++) {
                    NSString *key = keys[i];
                    NSObject *value = dic[key];
                    [desc appendString:@"    "];
                    [desc appendFormat:@"%@ = %@",key, XZJSONDescriptionWithIndent(XZJSONDescription(value).mutableCopy, 1)];
                    [desc appendString:(i + 1 == max) ? @"\n" : @";\n"];
                }
                [desc appendString:@"}"];
            }
            return desc;
        }
        
        default: {
            NSMutableString *desc = [NSMutableString new];
            [desc appendFormat:@"<%@: %p>", model.class, model];
            if (modelMeta->_properties.count == 0) return desc;
            
            // sort property names
            NSArray *properties = [modelMeta->_properties
                                   sortedArrayUsingComparator:^NSComparisonResult(XZJSONPropertyDescriptor *p1, XZJSONPropertyDescriptor *p2) {
                                       return [p1->_name compare:p2->_name];
                                   }];
            
            [desc appendFormat:@" {\n"];
            for (NSUInteger i = 0, max = properties.count; i < max; i++) {
                XZJSONPropertyDescriptor *property = properties[i];
                NSString *propertyDesc;
                if (property->_isCNumber) {
                    NSNumber *num = XZJSONEncodeNSNumberForProperty(model, property);
                    propertyDesc = num.stringValue;
                } else {
                    switch (property->_type & XZObjcTypeMask) {
                        case XZObjcTypeObject: {
                            id v = ((id (*)(id, SEL))(void *) objc_msgSend)((id)model, property->_getter);
                            propertyDesc = XZJSONDescription(v);
                            if (!propertyDesc) propertyDesc = @"<nil>";
                            break;
                        }
                        case XZObjcTypeClass: {
                            id v = ((id (*)(id, SEL))(void *) objc_msgSend)((id)model, property->_getter);
                            propertyDesc = ((NSObject *)v).description;
                            if (!propertyDesc) propertyDesc = @"<nil>";
                            break;
                        }
                        case XZObjcTypeSEL: {
                            SEL sel = ((SEL (*)(id, SEL))(void *) objc_msgSend)((id)model, property->_getter);
                            if (sel) propertyDesc = NSStringFromSelector(sel);
                            else propertyDesc = @"<NULL>";
                            break;
                        }
                        case XZObjcTypeBlock: {
                            id block = ((id (*)(id, SEL))(void *) objc_msgSend)((id)model, property->_getter);
                            propertyDesc = block ? ((NSObject *)block).description : @"<nil>";
                            break;
                        }
                        case XZObjcTypeCArray: case XZObjcTypeCString: case XZObjcTypePointer: {
                            void *pointer = ((void* (*)(id, SEL))(void *) objc_msgSend)((id)model, property->_getter);
                            propertyDesc = [NSString stringWithFormat:@"%p",pointer];
                            break;
                        }
                        case XZObjcTypeStruct: case XZObjcTypeUnion: {
                            NSValue *value = [model valueForKey:property->_name];
                            propertyDesc = value ? value.description : @"{unknown}";
                            break;
                        }
                        default: propertyDesc = @"<unknown>";
                    }
                }
                
                propertyDesc = XZJSONDescriptionWithIndent(propertyDesc.mutableCopy, 1);
                [desc appendFormat:@"    %@ = %@",property->_name, propertyDesc];
                [desc appendString:(i + 1 == max) ? @"\n" : @";\n"];
            }
            [desc appendFormat:@"}"];
            return desc;
        }
    }
}

