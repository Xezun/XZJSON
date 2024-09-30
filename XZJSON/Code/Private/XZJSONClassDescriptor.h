//
//  XZJSONClassDescriptor.h
//  XZJSON
//
//  Created by 徐臻 on 2024/9/29.
//

#import <Foundation/Foundation.h>
#import <XZJSON/XZObjcDescriptor.h>
#import <XZJSON/XZJSONDescriptor.h>

NS_ASSUME_NONNULL_BEGIN

/// A class info in object model.
@interface XZJSONClassDescriptor : NSObject {
    @package
    XZObjcClassDescriptor *_objcDescriptor;
    /// Key:mapped key and key path, Value:XZJSONObjcPropertyMeta.
    NSDictionary *_mapper;
    /// Array<XZJSONObjcPropertyMeta>, all property meta of this model.
    NSArray *_allPropertyMetas;
    /// Array<XZJSONObjcPropertyMeta>, property meta which is mapped to a key path.
    NSArray *_keyPathPropertyMetas;
    /// Array<XZJSONObjcPropertyMeta>, property meta which is mapped to multi keys.
    NSArray *_multiKeysPropertyMetas;
    /// The number of mapped key (and key path), same to _mapper.count.
    NSUInteger _keyMappedCount;
    /// Model class type.
    XZJSONEncodingNSType _nsType;
    
    BOOL _hasCustomWillTransformFromDictionary;
    BOOL _hasCustomTransformFromDictionary;
    BOOL _hasCustomTransformToDictionary;
    
    BOOL _supportsXZJSONDecoding;
    BOOL _forwardsDecodeForClass;   ///< 转发模型解析
    BOOL _canEncodeFromDictionary;
    BOOL _usesDecodingInitializer;
    
    BOOL _supportsXZJSONEncoding;
}
- (nullable instancetype)initWithClass:(Class)aClass NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

+ (nullable XZJSONClassDescriptor *)descriptorForClass:(nullable Class)aClass;
@end

NS_ASSUME_NONNULL_END
