//
//  Example001ViewController.m
//  Example
//
//  Created by Xezun on 2023/7/27.
//

#import "Example001ViewController.h"
#import "Example001Models.h"

@interface Example001ViewController ()
@end

@implementation Example001ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSURL *url = [NSBundle.mainBundle URLForResource:@"data" withExtension:@"json"];
    NSData *data = [NSData dataWithContentsOfURL:url];
    
    Example001Teacher *teacher = [XZJSON decode:data options:(NSJSONReadingAllowFragments) class:[Example001Teacher class]];
    
    NSAssert([teacher isKindOfClass:[Example001Teacher class]], @"");
    NSAssert([teacher.name isEqualToString:@"Smith"], @"");
    NSAssert(teacher.age == 50, @"");
    NSAssert(teacher.students.count == 3, @"");
    
    [teacher.students enumerateObjectsUsingBlock:^(Example001Student * _Nonnull student, NSUInteger idx, BOOL * _Nonnull stop) {
        NSAssert([student isKindOfClass:[Example001Student class]], @"");
        NSAssert([student.teacher isKindOfClass:[Example001Teacher class]], @"");
        if ([student.name isEqualToString:@"Peter"]) {
            NSAssert(student.age == 20, @"");
        } else if ([student.name isEqualToString:@"Jim"]) {
            NSAssert(student.age == 21, @"");
        } else if ([student.name isEqualToString:@"Lily"]) {
            NSAssert(student.age == 19, @"");
        } else {
            NSAssert(NO, @"teacher.students 校验失败");
        }
    }];

    NSData *json = [XZJSON encode:teacher options:NSJSONWritingPrettyPrinted error:nil];
    NSLog(@"%@", [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding]);
}


@end


