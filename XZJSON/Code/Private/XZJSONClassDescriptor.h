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

@class XZJSONPropertyDescriptor;

/// A class info in object model.
@interface XZJSONClassDescriptor : NSObject {
    @package
    /// 描述类信息的对象。
    XZObjcClassDescriptor *_descriptor;
    /// 所有可模型化或序列化的属性。Array, all property meta of this model.
    NSArray<XZJSONPropertyDescriptor *> *_properties;
    /// 所有可模型化或序列化的属性的数量。The number of mapped key (and key path), same to _mapper.count.
    NSUInteger _numberOfProperties;
    /// JSON 键与属性的映射。Key:mapped key and key path, Value:XZJSONObjcPropertyMeta.
    NSDictionary<NSString *, XZJSONPropertyDescriptor *> *_keyProperties;
    /// JSON 键值路径与属性的映射。Array<XZJSONObjcPropertyMeta>, property meta which is mapped to a key path.
    NSArray *_keyPathProperties;
    /// 多个 JSON 键与属性的映射。Array<XZJSONObjcPropertyMeta>, property meta which is mapped to multi keys.
    NSArray *_keyArrayProperties;
    /// 如果是，原生对象的类型。 Model class type.
    XZJSONEncodingNSType _nsType;
    
    /// 是否自定义模型化过程
    BOOL _supportsXZJSONDecoding;
    /// 是否需要转发模型解析
    BOOL _forwardsDecodeForClass;
    /// 是否校验数据
    BOOL _canEncodeFromDictionary;
    /// 是否使用初始化方法
    BOOL _usesDecodingInitializer;
    
    /// 是否自定义序列化过程
    BOOL _supportsXZJSONEncoding;
}

- (instancetype)init NS_UNAVAILABLE;
+ (nullable XZJSONClassDescriptor *)descriptorForClass:(nullable Class)aClass;

@end

NS_ASSUME_NONNULL_END
