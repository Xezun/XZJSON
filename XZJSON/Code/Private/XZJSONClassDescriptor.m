//
//  XZJSONClassDescriptor.m
//  XZJSON
//
//  Created by 徐臻 on 2024/9/29.
//

#import "XZJSONClassDescriptor.h"
#import "NSObject+YYModel.h"
#import "XZJSONPropertyDescriptor.h"
#import "XZJSONDefines.h"

@implementation XZJSONClassDescriptor

- (instancetype)initWithClass:(Class)aClass {
    XZObjcClassDescriptor * const objcDescriptor = [XZObjcClassDescriptor descriptorForClass:aClass];
    if (objcDescriptor == nil) {
        return nil;
    }
    self = [super init];
    if (self == nil) {
        return nil;
    }
    
    
    
    // Get black list
    NSSet *blacklist = nil;
    if ([aClass respondsToSelector:@selector(modelPropertyBlacklist)]) {
        NSArray *properties = [(id<YYModel>)aClass modelPropertyBlacklist];
        if (properties) {
            blacklist = [NSSet setWithArray:properties];
        }
    }
    
    // Get white list
    NSSet *whitelist = nil;
    if ([aClass respondsToSelector:@selector(modelPropertyWhitelist)]) {
        NSArray *properties = [(id<YYModel>)aClass modelPropertyWhitelist];
        if (properties) {
            whitelist = [NSSet setWithArray:properties];
        }
    }
    
    
    
    // Get container property's generic class
    NSDictionary *genericMapper = nil;
    if ([aClass respondsToSelector:@selector(modelContainerPropertyGenericClass)]) {
        genericMapper = [(id<YYModel>)aClass modelContainerPropertyGenericClass];
        if (genericMapper) {
            NSMutableDictionary *tmp = [NSMutableDictionary new];
            [genericMapper enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
                if (![key isKindOfClass:[NSString class]]) return;
                Class meta = object_getClass(obj);
                if (!meta) return;
                if (class_isMetaClass(meta)) {
                    tmp[key] = obj;
                } else if ([obj isKindOfClass:[NSString class]]) {
                    Class cls = NSClassFromString(obj);
                    if (cls) {
                        tmp[key] = cls;
                    }
                }
            }];
            genericMapper = tmp;
        }
    }
    
    // Create all property metas.
    NSMutableDictionary *allPropertyMetas = [NSMutableDictionary new];
    XZObjcClassDescriptor *curClassInfo = objcDescriptor;
    while (curClassInfo && curClassInfo.originSuperClass != nil) { // recursive parse super class, but ignore root class (NSObject/NSProxy)
        for (XZObjcPropertyDescriptor *propertyInfo in curClassInfo.properties.allValues) {
            if (!propertyInfo.name) continue;
            if (blacklist && [blacklist containsObject:propertyInfo.name]) continue;
            if (whitelist && ![whitelist containsObject:propertyInfo.name]) continue;
            XZJSONPropertyDescriptor *meta = [XZJSONPropertyDescriptor descriptorWithClass:objcDescriptor
                                                                    property:propertyInfo
                                                                         elementClass:genericMapper[propertyInfo.name]];
            if (!meta || !meta->_name) continue;
            if (!meta->_getter || !meta->_setter) continue;
            if (allPropertyMetas[meta->_name]) continue;
            allPropertyMetas[meta->_name] = meta;
        }
        curClassInfo = curClassInfo.superClassDescriptor;
    }
    if (allPropertyMetas.count) _allPropertyMetas = allPropertyMetas.allValues.copy;
    
    // create mapper
    NSMutableDictionary *mapper = [NSMutableDictionary new];
    NSMutableArray *keyPathPropertyMetas = [NSMutableArray new];
    NSMutableArray *multiKeysPropertyMetas = [NSMutableArray new];
    
    if ([aClass respondsToSelector:@selector(modelCustomPropertyMapper)]) {
        NSDictionary *customMapper = [(id <YYModel>)aClass modelCustomPropertyMapper];
        [customMapper enumerateKeysAndObjectsUsingBlock:^(NSString *propertyName, NSString *mappedToKey, BOOL *stop) {
            XZJSONPropertyDescriptor *propertyMeta = allPropertyMetas[propertyName];
            if (!propertyMeta) return;
            [allPropertyMetas removeObjectForKey:propertyName];
            
            if ([mappedToKey isKindOfClass:[NSString class]]) {
                if (mappedToKey.length == 0) return;
                
                propertyMeta->_mappedToKey = mappedToKey;
                NSArray *keyPath = [mappedToKey componentsSeparatedByString:@"."];
                for (NSString *onePath in keyPath) {
                    if (onePath.length == 0) {
                        NSMutableArray *tmp = keyPath.mutableCopy;
                        [tmp removeObject:@""];
                        keyPath = tmp;
                        break;
                    }
                }
                if (keyPath.count > 1) {
                    propertyMeta->_mappedToKeyPath = keyPath;
                    [keyPathPropertyMetas addObject:propertyMeta];
                }
                propertyMeta->_next = mapper[mappedToKey] ?: nil;
                mapper[mappedToKey] = propertyMeta;
                
            } else if ([mappedToKey isKindOfClass:[NSArray class]]) {
                
                NSMutableArray *mappedToKeyArray = [NSMutableArray new];
                for (NSString *oneKey in ((NSArray *)mappedToKey)) {
                    if (![oneKey isKindOfClass:[NSString class]]) continue;
                    if (oneKey.length == 0) continue;
                    
                    NSArray *keyPath = [oneKey componentsSeparatedByString:@"."];
                    if (keyPath.count > 1) {
                        [mappedToKeyArray addObject:keyPath];
                    } else {
                        [mappedToKeyArray addObject:oneKey];
                    }
                    
                    if (!propertyMeta->_mappedToKey) {
                        propertyMeta->_mappedToKey = oneKey;
                        propertyMeta->_mappedToKeyPath = keyPath.count > 1 ? keyPath : nil;
                    }
                }
                if (!propertyMeta->_mappedToKey) return;
                
                propertyMeta->_mappedToKeyArray = mappedToKeyArray;
                [multiKeysPropertyMetas addObject:propertyMeta];
                
                propertyMeta->_next = mapper[mappedToKey] ?: nil;
                mapper[mappedToKey] = propertyMeta;
            }
        }];
    }
    
    [allPropertyMetas enumerateKeysAndObjectsUsingBlock:^(NSString *name, XZJSONPropertyDescriptor *propertyMeta, BOOL *stop) {
        propertyMeta->_mappedToKey = name;
        propertyMeta->_next = mapper[name] ?: nil;
        mapper[name] = propertyMeta;
    }];
    
    if (mapper.count) _mapper = mapper;
    if (keyPathPropertyMetas) _keyPathPropertyMetas = keyPathPropertyMetas;
    if (multiKeysPropertyMetas) _multiKeysPropertyMetas = multiKeysPropertyMetas;
    
    _objcDescriptor = objcDescriptor;
    _keyMappedCount = _allPropertyMetas.count;
    _nsType = XZJSONEncodingNSTypeFromClass(aClass);
    _hasCustomWillTransformFromDictionary = ([aClass instancesRespondToSelector:@selector(modelCustomWillTransformFromDictionary:)]);
    _hasCustomTransformFromDictionary     = ([aClass instancesRespondToSelector:@selector(modelCustomTransformFromDictionary:)]);
    _hasCustomTransformToDictionary       = ([aClass instancesRespondToSelector:@selector(modelCustomTransformToDictionary:)]);
    
    _supportsXZJSONDecoding = [aClass conformsToProtocol:@protocol(XZJSONDecoding)];
    _forwardsDecodeForClass = (_supportsXZJSONDecoding && [aClass respondsToSelector:@selector(forwardingClassForJSONDictionary:)]);
    
    _supportsXZJSONEncoding = [aClass conformsToProtocol:@protocol(XZJSONDecoding)];
    
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
    
    if (!descriptor || descriptor->_objcDescriptor.isValid) {
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
