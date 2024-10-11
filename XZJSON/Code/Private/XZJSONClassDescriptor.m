//
//  XZJSONClassDescriptor.m
//  XZJSON
//
//  Created by 徐臻 on 2024/9/29.
//

#import "XZJSONClassDescriptor.h"
#import "XZJSONPropertyDescriptor.h"
#import "XZJSONDefines.h"

@implementation XZJSONClassDescriptor

- (instancetype)initWithClass:(nonnull Class)rawClass {
    XZObjcClassDescriptor * const aClass = [XZObjcClassDescriptor descriptorForClass:rawClass];
    if (aClass == nil) {
        return nil;
    }
    self = [super init];
    if (self == nil) {
        return nil;
    }
    
    // 黑名单
    NSSet *blockedKeys = nil;
    if ([rawClass respondsToSelector:@selector(blockedJSONCodingKeys)]) {
        NSArray *properties = [rawClass blockedJSONCodingKeys];
        if (properties) {
            blockedKeys = [NSSet setWithArray:properties];
        }
    }
    
    // 白名单
    NSSet *allowedKeys = nil;
    if ([rawClass respondsToSelector:@selector(allowedJSONCodingKeys)]) {
        NSArray *properties = [rawClass allowedJSONCodingKeys];
        if (properties) {
            allowedKeys = [NSSet setWithArray:properties];
        }
    }
    
    // 类映射
    NSDictionary *mappingClasses = nil;
    if ([rawClass respondsToSelector:@selector(mappingJSONCodingClasses)]) {
        mappingClasses = [rawClass mappingJSONCodingClasses];
        if (mappingClasses) {
            NSMutableDictionary *tmp = [NSMutableDictionary new];
            [mappingClasses enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
                if (![key isKindOfClass:[NSString class]]) return;
                if (object_isClass(obj)) {
                    tmp[key] = obj;
                } else if ([obj isKindOfClass:[NSString class]]) {
                    tmp[key] = NSClassFromString(obj);
                }
            }];
            mappingClasses = tmp;
        }
    }
    
    // Create all property metas.
    NSMutableDictionary * const namedProperties = [NSMutableDictionary new];
    XZObjcClassDescriptor *currentClass = aClass;
    do {
        [currentClass.properties enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull name, XZObjcPropertyDescriptor * _Nonnull property, BOOL * _Nonnull stop) {
            if (blockedKeys && [blockedKeys containsObject:name])     {
                return; // 有黑名单，则不能在黑名单中
            }
            if (allowedKeys && ![allowedKeys containsObject:name])     {
                return; // 有白名单，则必须在白名单中
            }
            if (namedProperties[name]) {
                return; // 已存在
            }
            XZJSONPropertyDescriptor *descriptor = [XZJSONPropertyDescriptor descriptorWithClass:aClass property:property elementClass:mappingClasses[property.name]];
            if (!descriptor->_getter || !descriptor->_setter) {
                return;
            }
            namedProperties[name] = descriptor;
        }];
    } while ((currentClass = currentClass.superDescriptor));
    if (namedProperties.count) _properties = namedProperties.allValues.copy;
    
    // create mapper
    NSMutableDictionary *keyProperties      = [NSMutableDictionary new];
    NSMutableArray      *keyPathProperties  = [NSMutableArray new];
    NSMutableArray      *keyArrayProperties = [NSMutableArray new];
    
    if ([rawClass respondsToSelector:@selector(mappingJSONCodingKeys)]) {
        [[rawClass mappingJSONCodingKeys] enumerateKeysAndObjectsUsingBlock:^(NSString * const propertyName, id const JSONKey, BOOL *stop) {
            XZJSONPropertyDescriptor *property = namedProperties[propertyName];
            if (!property) return;
            // TODO: 测试 JSONKey 不合法时，该属性是否会被忽略
            [namedProperties removeObjectForKey:propertyName];
            
            if ([JSONKey isKindOfClass:[NSString class]]) {
                NSString * const stringKey = JSONKey;
                if (stringKey.length == 0) return;
                
                property->_JSONKey = stringKey;
                
                if ([stringKey containsString:@"."]) {
                    NSArray *keyPath = [stringKey componentsSeparatedByString:@"."];
                    while ([keyPath containsObject:@""]) {
                        NSMutableArray *arrayM = (NSMutableArray *)keyPath;
                        if (![arrayM isKindOfClass:NSMutableArray.class]) {
                            arrayM = [keyPath mutableCopy];
                            keyPath = arrayM;
                        }
                        [arrayM removeObject:@""];
                    }
                    
                    if (keyPath.count > 1) {
                        property->_JSONKeyPath = keyPath;
                        [keyPathProperties addObject:property];
                    }
                }
                
                property->_next = keyProperties[stringKey];
                keyProperties[stringKey] = property;
            } else if ([JSONKey isKindOfClass:[NSArray class]]) {
                NSArray * const arrayKey = JSONKey;
                
                NSMutableArray *JSONKeyArray = [NSMutableArray new];
                for (NSString *key in arrayKey) {
                    if (![key isKindOfClass:[NSString class]]) continue;
                    if (key.length == 0) continue;
                    
                    NSArray *keyPath = [key componentsSeparatedByString:@"."];
                    while ([keyPath containsObject:@""]) {
                        NSMutableArray *arrayM = (NSMutableArray *)keyPath;
                        if (![arrayM isKindOfClass:NSMutableArray.class]) {
                            arrayM = [keyPath mutableCopy];
                            keyPath = arrayM;
                        }
                        [arrayM removeObject:@""];
                    }
                    
                    if (keyPath.count > 1) {
                        [JSONKeyArray addObject:keyPath];
                    } else {
                        [JSONKeyArray addObject:key];
                    }
                    
                    // 数组中的第一个 key
                    if (!property->_JSONKey) {
                        property->_JSONKey = key;
                        property->_JSONKeyPath = keyPath.count > 1 ? keyPath : nil;
                    }
                    
                    // TODO: YYModel 源代码有问题，此处待验证
                    property->_next = keyProperties[key];
                    keyProperties[key] = property;
                }
                if (!property->_JSONKey) return;
                
                property->_JSONKeyArray = JSONKeyArray;
                [keyArrayProperties addObject:property];
            }
        }];
    }
    
    [namedProperties enumerateKeysAndObjectsUsingBlock:^(NSString *name, XZJSONPropertyDescriptor *property, BOOL *stop) {
        property->_JSONKey = name;
        property->_next = keyProperties[name] ?: nil;
        keyProperties[name] = property;
    }];
    
    if (keyProperties.count)    _keyProperties      = keyProperties;
    if (keyPathProperties)      _keyPathProperties  = keyPathProperties;
    if (keyArrayProperties)     _keyArrayProperties = keyArrayProperties;
    
    _class = aClass;
    _numberOfProperties = _properties.count;
    _nsType = XZJSONEncodingNSTypeFromClass(rawClass);
    
    BOOL const _supportsXZJSONCoding = [rawClass conformsToProtocol:@protocol(XZJSONCoding)];
    _forwardsDecodeForClass = (_supportsXZJSONCoding && [rawClass respondsToSelector:@selector(forwardingClassForJSONDictionary:)]);
    _canEncodeFromDictionary = (_supportsXZJSONCoding && [rawClass respondsToSelector:@selector(canDecodeFromJSONDictionary:)]);
    
    BOOL const _supportsXZJSONDecoding = [rawClass conformsToProtocol:@protocol(XZJSONDecoding)];
    _usesDecodingInitializer = (_supportsXZJSONDecoding && [rawClass instancesRespondToSelector:@selector(initWithJSONDictionary:)]);
    
    _supportsXZJSONEncoding = [rawClass conformsToProtocol:@protocol(XZJSONDecoding)] && [rawClass instancesRespondToSelector:@selector(encodeIntoJSONDictionary:)];
    
    return self;
}

+ (XZJSONClassDescriptor *)descriptorForClass:(Class)aClass {
    if (aClass == Nil || !object_isClass(aClass) || [aClass superclass] == Nil || class_isMetaClass(aClass)) {
        return nil;
    }
    
    static CFMutableDictionaryRef _cachedDescriptors;
    static dispatch_semaphore_t   _lock;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _cachedDescriptors = CFDictionaryCreateMutable(CFAllocatorGetDefault(), 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        _lock = dispatch_semaphore_create(1);
    });
    
    dispatch_semaphore_wait(_lock, DISPATCH_TIME_FOREVER);
    XZJSONClassDescriptor *descriptor = CFDictionaryGetValue(_cachedDescriptors, (__bridge const void *)(aClass));
    dispatch_semaphore_signal(_lock);
    
    if (!descriptor || descriptor->_class.isValid) {
        descriptor = [[XZJSONClassDescriptor alloc] initWithClass:aClass];
        if (descriptor) {
            dispatch_semaphore_wait(_lock, DISPATCH_TIME_FOREVER);
            CFDictionarySetValue(_cachedDescriptors, (__bridge const void *)(aClass), (__bridge const void *)(descriptor));
            dispatch_semaphore_signal(_lock);
        }
    }
    
    return descriptor;
}

@end
