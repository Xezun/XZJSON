//
//  XZJSON.m
//  XZJSON
//
//  Created by 徐臻 on 2024/9/28.
//

#import "XZJSON.h"
#import "NSObject+YYModel.h"
#import "XZJSONClassDescriptor.h"
#import "XZJSONPrivate.h"

@implementation XZJSON

#pragma mark - Decoder

+ (id)decode:(id)json options:(NSJSONReadingOptions)options class:(Class)aClass {
    if (json == nil || json == NSNull.null) {
        return nil;
    }
    // 二进制流形式的 json 数据。
    if ([json isKindOfClass:NSData.class]) {
        return [self decodeData:json options:options class:aClass];
    }
    // 字符串形式的 json 数据。
    if ([json isKindOfClass:NSString.class]) {
        NSData * const data = [((NSString *)json) dataUsingEncoding:NSUTF8StringEncoding];
        if (data == nil) {
            return nil;
        }
        return [self decodeData:data options:options class:aClass];
    }
    // 默认数组为，解析多个 json 数据
    if ([json isKindOfClass:NSArray.class]) {
        NSMutableArray *arrayM = [NSMutableArray arrayWithCapacity:((NSArray *)json).count];
        for (id object in json) {
            id const model = [self decode:object options:options class:aClass];
            if (model != nil) {
                [arrayM addObject:model];
            }
        }
        return arrayM;
    }
    // 其它情况视为已解析好的 json
    return [self decodeObject:json class:aClass];
}

+ (nullable id)decodeData:(nonnull NSData *)data options:(NSJSONReadingOptions)options class:(Class)aClass {
    NSError *error = nil;
    id const object = [NSJSONSerialization JSONObjectWithData:data options:options error:&error];
    if (error == nil || error.code == noErr ) {
        return [self decodeObject:object class:aClass];
    }
    return nil;
}

+ (nullable id)decodeObject:(nonnull id)object class:(Class)aClass {
    if (object == NSNull.null) {
        return nil;
    }
    if ([object isKindOfClass:NSDictionary.class]) {
        XZJSONClassDescriptor * const descriptor = [XZJSONClassDescriptor descriptorForClass:aClass];
        
        if (descriptor->_supportsXZJSONDecoding) {
            if (descriptor->_forwardsDecodeForClass) {
                aClass = [aClass forwardingClassForJSONDictionary:object];
                if (aClass == Nil) {
                    return nil;
                }
            }
            
            if (descriptor->_canEncodeFromDictionary) {
                object = [aClass canDecodeFromJSONDictionary:object];
                if (object == nil) {
                    return nil;
                }
            }
            
            if (descriptor->_usesDecodingInitializer) {
                return [[aClass alloc] initWithJSONDictionary:object];
            }
        }
        
        id model = [aClass new];
        if (model != nil) {
            [self object:model decodeWithDictionary:object];
        }
        return model;
    }
    if ([object isKindOfClass:NSArray.class]) {
        NSArray * const array = object;
        if (array.count == 0) {
            return array;
        }
        
        NSMutableArray * const arrayM = [NSMutableArray arrayWithCapacity:array.count];
        for (id item in array) {
            id const model = [self decodeObject:item class:aClass];
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

#pragma mark - Encoder

+ (NSData *)encode:(id)object options:(NSJSONWritingOptions)options error:(NSError *__autoreleasing  _Nullable * _Nullable)error {
    if (object == nil) {
        return nil;
    }
    
    id const JSONObject = [self encodeObject:object];
    
    if (JSONObject == nil) {
        return nil;
    }
    
    return [NSJSONSerialization dataWithJSONObject:JSONObject options:options error:error];
}

/// 返回 JSONObject
+ (id)encodeObject:(id)object {
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
            id obj = [self encodeObject:item];
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
            obj = [self encodeObject:obj];
            if (obj != nil) {
                dictM[key] = obj;
            }
        }];
        return dictM;
    }
    // TODO: - opt
    if ([object conformsToProtocol:@protocol(XZJSONEncoding)]) {
        return [(id<XZJSONEncoding>)object encodeIntoJSONDictionary];
    }
    NSMutableDictionary *dictionary = [NSMutableDictionary dictionary];
    [self object:object encodeIntoDictionary:dictionary];
    return dictionary;
}

#pragma mark - Encoding or Decoding

+ (void)object:(id)object decodeWithDictionary:(NSDictionary *)dictionary {
    XZJSONClassDescriptor * const descriptor = [XZJSONClassDescriptor descriptorForClass:[object class]];
    [self object:object decodeWithDictionary:dictionary descriptor:descriptor];
}

// yy_modelSetWithDictionary
+ (void)object:(id)object decodeWithDictionary:(NSDictionary *)dictionary descriptor:(XZJSONClassDescriptor *)modelMeta {
    if (modelMeta->_keyMappedCount == 0) return;
   
    XZJSONEncodingContext context = {0};
    context.descriptor = (__bridge void *)(modelMeta);
    context.model      = (__bridge void *)(object);
    context.dictionary = (__bridge void *)(dictionary);
    
    if (modelMeta->_keyMappedCount >= CFDictionaryGetCount((CFDictionaryRef)dictionary)) {
        CFDictionaryApplyFunction((CFDictionaryRef)dictionary, XZJSONDecodingDictionaryEnumeratorFunction, &context);
        if (modelMeta->_keyPathPropertyMetas) {
            CFArrayApplyFunction((CFArrayRef)modelMeta->_keyPathPropertyMetas,
                                 CFRangeMake(0, CFArrayGetCount((CFArrayRef)modelMeta->_keyPathPropertyMetas)),
                                 XZJSONDecodingArrayEnumeratorFunction,
                                 &context);
        }
        if (modelMeta->_multiKeysPropertyMetas) {
            CFArrayApplyFunction((CFArrayRef)modelMeta->_multiKeysPropertyMetas,
                                 CFRangeMake(0, CFArrayGetCount((CFArrayRef)modelMeta->_multiKeysPropertyMetas)),
                                 XZJSONDecodingArrayEnumeratorFunction,
                                 &context);
        }
    } else {
        CFArrayApplyFunction((CFArrayRef)modelMeta->_allPropertyMetas,
                             CFRangeMake(0, modelMeta->_keyMappedCount),
                             XZJSONDecodingArrayEnumeratorFunction,
                             &context);
    }
}

+ (void)object:(id)object encodeIntoDictionary:(NSMutableDictionary *)dictionary {
    XZJSONEncodingRecursive(object);
}

@end



@implementation XZJSON (NSCoder)

// - (void)yy_modelEncodeWithCoder:(NSCoder *)aCoder
+ (void)object:(id)object encodeWithCoder:(NSCoder *)aCoder {
    if (!aCoder) return;
    if (object == (id)kCFNull) {
        [((id<NSCoding>)object)encodeWithCoder:aCoder];
        return;
    }
    
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:[object class]];
    if (modelMeta->_nsType) {
        [((id<NSCoding>)object)encodeWithCoder:aCoder];
        return;
    }
    
    for (XZJSONPropertyDescriptor *propertyMeta in modelMeta->_allPropertyMetas) {
        if (!propertyMeta->_getter) return;
        
        if (propertyMeta->_isCNumber) {
            NSNumber *value = ModelCreateNumberFromProperty(object, propertyMeta);
            if (value) [aCoder encodeObject:value forKey:propertyMeta->_name];
        } else {
            switch (propertyMeta->_type & XZObjcTypeMask) {
                case XZObjcTypeObject: {
                    id value = ((id (*)(id, SEL))(void *)objc_msgSend)((id)object, propertyMeta->_getter);
                    if (value && (propertyMeta->_nsType || [value respondsToSelector:@selector(encodeWithCoder:)])) {
                        if ([value isKindOfClass:[NSValue class]]) {
                            if ([value isKindOfClass:[NSNumber class]]) {
                                [aCoder encodeObject:value forKey:propertyMeta->_name];
                            }
                        } else {
                            [aCoder encodeObject:value forKey:propertyMeta->_name];
                        }
                    }
                } break;
                case XZObjcTypeSEL: {
                    SEL value = ((SEL (*)(id, SEL))(void *)objc_msgSend)((id)object, propertyMeta->_getter);
                    if (value) {
                        NSString *str = NSStringFromSelector(value);
                        [aCoder encodeObject:str forKey:propertyMeta->_name];
                    }
                } break;
                case XZObjcTypeStruct:
                case XZObjcTypeUnion: {
                    if (propertyMeta->_isKVCCompatible && propertyMeta->_isStructAvailableForKeyedArchiver) {
                        @try {
                            NSValue *value = [object valueForKey:NSStringFromSelector(propertyMeta->_getter)];
                            [aCoder encodeObject:value forKey:propertyMeta->_name];
                        } @catch (NSException *exception) {}
                    }
                } break;
                    
                default:
                    break;
            }
        }
    }
}

// - (id)yy_modelInitWithCoder:(NSCoder *)aDecoder
+ (id)object:(id)object decodeWithCoder:(NSCoder *)aDecoder {
    if (!aDecoder) return object;
    if (object == (id)kCFNull) return object;
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:[object class]];
    if (modelMeta->_nsType) return object;
    
    for (XZJSONPropertyDescriptor *propertyMeta in modelMeta->_allPropertyMetas) {
        if (!propertyMeta->_setter) continue;
        
        if (propertyMeta->_isCNumber) {
            NSNumber *value = [aDecoder decodeObjectForKey:propertyMeta->_name];
            if ([value isKindOfClass:[NSNumber class]]) {
                ModelSetNumberToProperty(object, value, propertyMeta);
                [value class];
            }
        } else {
            XZObjcType type = propertyMeta->_type & XZObjcTypeMask;
            switch (type) {
                case XZObjcTypeObject: {
                    id value = [aDecoder decodeObjectForKey:propertyMeta->_name];
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)object, propertyMeta->_setter, value);
                } break;
                case XZObjcTypeSEL: {
                    NSString *str = [aDecoder decodeObjectForKey:propertyMeta->_name];
                    if ([str isKindOfClass:[NSString class]]) {
                        SEL sel = NSSelectorFromString(str);
                        ((void (*)(id, SEL, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_setter, sel);
                    }
                } break;
                case XZObjcTypeStruct:
                case XZObjcTypeUnion: {
                    if (propertyMeta->_isKVCCompatible) {
                        @try {
                            NSValue *value = [aDecoder decodeObjectForKey:propertyMeta->_name];
                            if (value) [object setValue:value forKey:propertyMeta->_name];
                        } @catch (NSException *exception) {}
                    }
                } break;
                    
                default:
                    break;
            }
        }
    }
    return object;
}

+ (NSUInteger)objectHash:(id)object {
    if (object == (id)kCFNull) return [object hash];
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:[object class]];
    if (modelMeta->_nsType) return [object hash];
    
    NSMutableString *string = [NSMutableString string];
    for (XZJSONPropertyDescriptor *propertyMeta in modelMeta->_allPropertyMetas) {
        if (!propertyMeta->_isKVCCompatible) continue;
        
        NSUInteger const hash = [[object valueForKey:NSStringFromSelector(propertyMeta->_getter)] hash];
        [string appendFormat:@"%lu", hash];
    }
    
    if (string.length == 0) {
        return (long)((__bridge void *)object);
    }
    return string.hash;
}

+ (BOOL)object:(id)object1 isEqualToObject:(id)object2 {
    if (object1 == object2) return YES;
    if (![object2 isMemberOfClass:[object1 class]]) return NO;
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:[object1 class]];
    if (modelMeta->_nsType) return [object1 isEqual:object2];
    if ([object1 hash] != [object2 hash]) return NO;
    
    for (XZJSONPropertyDescriptor *propertyMeta in modelMeta->_allPropertyMetas) {
        if (!propertyMeta->_isKVCCompatible) continue;
        id this = [object1 valueForKey:NSStringFromSelector(propertyMeta->_getter)];
        id that = [object2 valueForKey:NSStringFromSelector(propertyMeta->_getter)];
        if (this == that) continue;
        if (this == nil || that == nil) return NO;
        if (![this isEqual:that]) return NO;
    }
    return YES;
}

+ (BOOL)objectDescription:(id)object {
    return ModelDescription(object);
}

+ (id)objectCopy:(id)object {
    if (object == (id)kCFNull) return object;
    XZJSONClassDescriptor *modelMeta = [XZJSONClassDescriptor descriptorForClass:[object class]];
    if (modelMeta->_nsType) return [object copy];
    
    NSObject *one = [[object class] new];
    for (XZJSONPropertyDescriptor *propertyMeta in modelMeta->_allPropertyMetas) {
        if (!propertyMeta->_getter || !propertyMeta->_setter) continue;
        
        if (propertyMeta->_isCNumber) {
            switch (propertyMeta->_type & XZObjcTypeMask) {
                case XZObjcTypeBool: {
                    bool num = ((bool (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, bool))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeInt8:
                case XZObjcTypeUInt8: {
                    uint8_t num = ((bool (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, uint8_t))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeInt16:
                case XZObjcTypeUInt16: {
                    uint16_t num = ((uint16_t (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, uint16_t))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeInt32:
                case XZObjcTypeUInt32: {
                    uint32_t num = ((uint32_t (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, uint32_t))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeInt64:
                case XZObjcTypeUInt64: {
                    uint64_t num = ((uint64_t (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, uint64_t))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeFloat: {
                    float num = ((float (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, float))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeDouble: {
                    double num = ((double (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, double))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } break;
                case XZObjcTypeLongDouble: {
                    long double num = ((long double (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, long double))(void *) objc_msgSend)((id)one, propertyMeta->_setter, num);
                } // break; commented for code coverage in next line
                default: break;
            }
        } else {
            switch (propertyMeta->_type & XZObjcTypeMask) {
                case XZObjcTypeObject:
                case XZObjcTypeClass:
                case XZObjcTypeBlock: {
                    id value = ((id (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, id))(void *) objc_msgSend)((id)one, propertyMeta->_setter, value);
                } break;
                case XZObjcTypeSEL:
                case XZObjcTypePointer:
                case XZObjcTypeCString: {
                    size_t value = ((size_t (*)(id, SEL))(void *) objc_msgSend)((id)object, propertyMeta->_getter);
                    ((void (*)(id, SEL, size_t))(void *) objc_msgSend)((id)one, propertyMeta->_setter, value);
                } break;
                case XZObjcTypeStruct:
                case XZObjcTypeUnion: {
                    @try {
                        NSValue *value = [object valueForKey:NSStringFromSelector(propertyMeta->_getter)];
                        if (value) {
                            [one setValue:value forKey:propertyMeta->_name];
                        }
                    } @catch (NSException *exception) {}
                } // break; commented for code coverage in next line
                default: break;
            }
        }
    }
    return one;
}

@end
