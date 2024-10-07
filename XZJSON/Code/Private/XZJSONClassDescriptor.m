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

- (instancetype)initWithClass:(Class)aClass {
    XZObjcClassDescriptor * const descriptor = [XZObjcClassDescriptor descriptorForClass:aClass];
    if (descriptor == nil) {
        return nil;
    }
    self = [super init];
    if (self == nil) {
        return nil;
    }
    
    // 黑名单
    NSSet *blockedKeys = nil;
    if ([aClass respondsToSelector:@selector(blockedJSONCodingKeys)]) {
        NSArray *properties = [aClass blockedJSONCodingKeys];
        if (properties) {
            blockedKeys = [NSSet setWithArray:properties];
        }
    }
    
    // 白名单
    NSSet *allowedKeys = nil;
    if ([aClass respondsToSelector:@selector(allowedJSONCodingKeys)]) {
        NSArray *properties = [aClass allowedJSONCodingKeys];
        if (properties) {
            allowedKeys = [NSSet setWithArray:properties];
        }
    }
    
    // 类映射
    NSDictionary *mappingClasses = nil;
    if ([aClass respondsToSelector:@selector(mappingJSONCodingClasses)]) {
        mappingClasses = [aClass mappingJSONCodingClasses];
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
    NSMutableDictionary *allProperties = [NSMutableDictionary new];
    XZObjcClassDescriptor *currentDescriptor = descriptor;
    while (currentDescriptor && currentDescriptor.super != nil) { // recursive parse super class, but ignore root class (NSObject/NSProxy)
        for (XZObjcPropertyDescriptor *property in currentDescriptor.properties.allValues) {
            if (!property.name) continue;
            if (blockedKeys && [blockedKeys containsObject:property.name]) continue;
            if (allowedKeys && ![allowedKeys containsObject:property.name]) continue;
            XZJSONPropertyDescriptor *propertyDescriptor = [XZJSONPropertyDescriptor descriptorWithClass:descriptor property:property elementClass:mappingClasses[property.name]];
            if (!propertyDescriptor || !propertyDescriptor->_name) continue;
            if (!propertyDescriptor->_getter || !propertyDescriptor->_setter) continue;
            if (allProperties[propertyDescriptor->_name]) continue;
            allProperties[propertyDescriptor->_name] = propertyDescriptor;
        }
        currentDescriptor = currentDescriptor.super;
    }
    if (allProperties.count) _properties = allProperties.allValues.copy;
    
    // create mapper
    NSMutableDictionary *keyProperties    = [NSMutableDictionary new];
    NSMutableArray      *keyPathProperties  = [NSMutableArray new];
    NSMutableArray      *keyArrayProperties = [NSMutableArray new];
    
    if ([aClass respondsToSelector:@selector(mappingJSONCodingKeys)]) {
        NSDictionary *customMapper = [aClass mappingJSONCodingKeys];
        [customMapper enumerateKeysAndObjectsUsingBlock:^(NSString *propertyName, id const JSONKey, BOOL *stop) {
            XZJSONPropertyDescriptor *property = allProperties[propertyName];
            if (!property) return;
            [allProperties removeObjectForKey:propertyName];
            
            if ([JSONKey isKindOfClass:[NSString class]]) {
                NSString * const stringJSONKey = JSONKey;
                if (stringJSONKey.length == 0) return;
                
                property->_JSONKey = stringJSONKey;
                
                if ([stringJSONKey containsString:@"."]) {
                    NSArray *keyPath = [stringJSONKey componentsSeparatedByString:@"."];
                    if ([keyPath containsObject:@""]) {
                        NSMutableArray *arrayM = [keyPath mutableCopy];
                        [arrayM removeObject:@""];
                        keyPath = arrayM;
                    }
                    if (keyPath.count > 1) {
                        property->_JSONKeyPath = keyPath;
                        [keyPathProperties addObject:property];
                    }
                }
                
                property->_next = keyProperties[stringJSONKey];
                keyProperties[stringJSONKey] = property;
            } else if ([JSONKey isKindOfClass:[NSArray class]]) {
                
                NSMutableArray *JSONKeyArray = [NSMutableArray new];
                for (NSString *key in ((NSArray *)JSONKey)) {
                    if (![key isKindOfClass:[NSString class]]) continue;
                    if (key.length == 0) continue;
                    
                    NSArray *keyPath = [key componentsSeparatedByString:@"."];
                    if ([keyPath containsObject:@""]) {
                        NSMutableArray *arrayM = [keyPath mutableCopy];
                        [arrayM removeObject:@""];
                        keyPath = arrayM;
                    }
                    
                    if (keyPath.count > 1) {
                        [JSONKeyArray addObject:keyPath];
                    } else {
                        [JSONKeyArray addObject:key];
                    }
                    
                    if (!property->_JSONKey) {
                        property->_JSONKey = key;
                        property->_JSONKeyPath = keyPath.count > 1 ? keyPath : nil;
                    }
                }
                if (!property->_JSONKey) return;
                
                property->_JSONKeyArray = JSONKeyArray;
                [keyArrayProperties addObject:property];
                
                property->_next = keyProperties[JSONKey];
                keyProperties[JSONKey] = property;
            }
        }];
    }
    
    [allProperties enumerateKeysAndObjectsUsingBlock:^(NSString *name, XZJSONPropertyDescriptor *propertyMeta, BOOL *stop) {
        propertyMeta->_JSONKey = name;
        propertyMeta->_next = keyProperties[name] ?: nil;
        keyProperties[name] = propertyMeta;
    }];
    
    if (keyProperties.count)    _keyProperties      = keyProperties;
    if (keyPathProperties)      _keyPathProperties  = keyPathProperties;
    if (keyArrayProperties)     _keyArrayProperties = keyArrayProperties;
    
    _descriptor = descriptor;
    _keyMappedCount = _properties.count;
    _nsType = XZJSONEncodingNSTypeFromClass(aClass);
    
    _supportsXZJSONDecoding = [aClass conformsToProtocol:@protocol(XZJSONDecoding)];
    _forwardsDecodeForClass = (_supportsXZJSONDecoding && [aClass respondsToSelector:@selector(forwardingClassForJSONDictionary:)]);
    _canEncodeFromDictionary = (_supportsXZJSONDecoding && [aClass respondsToSelector:@selector(canDecodeFromJSONDictionary:)]);
    _usesDecodingInitializer = (_supportsXZJSONDecoding && [aClass instancesRespondToSelector:@selector(initWithJSONDictionary:)]);
    
    _supportsXZJSONEncoding = [aClass conformsToProtocol:@protocol(XZJSONDecoding)] && [aClass instancesRespondToSelector:@selector(encodeIntoJSONDictionary:)];
    
    return self;
}

+ (XZJSONClassDescriptor *)descriptorForClass:(Class)aClass {
    if (aClass == Nil) {
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
    
    if (!descriptor || descriptor->_descriptor.isValid) {
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
