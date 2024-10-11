//
//  Example001Models.m
//  Example
//
//  Created by 徐臻 on 2024/10/12.
//

#import "Example001Models.h"

@implementation Example001Human

+ (NSDictionary<NSString *,id> *)mappingJSONCodingKeys {
    return @{
        @"identifier": @"id"
    };
}

@end

@implementation Example001Teacher

+ (NSDictionary<NSString *,id> *)mappingJSONCodingClasses {
    return @{
        @"students": [Example001Student class]
    };
}

- (instancetype)initWithJSONDictionary:(NSDictionary *)JSON {
    [XZJSON object:self decodeWithDictionary:JSON];
    
    [self.students enumerateObjectsUsingBlock:^(Example001Student * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        obj.teacher = self;
    }];
    
    return self;
}

@end

@implementation Example001Student

+ (NSArray<NSString *> *)blockedJSONCodingKeys {
    return @[@"teacher"];
}

+ (NSArray<NSString *> *)allowedJSONCodingKeys {
    return nil;
}

@end

