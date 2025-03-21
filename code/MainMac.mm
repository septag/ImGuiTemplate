// Dear ImGui: standalone example application for OSX + Metal.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "Core.h"

#if PLATFORM_APPLE

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_metal.h"
#include "ImGui/imgui_impl_osx.h"
#include "ImGui/ImGuiMain.h"

#include "Main.h"

id <MTLDevice> gMetalDevice;

@interface AppViewController : NSViewController<NSWindowDelegate>
@end

@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@end

//-----------------------------------------------------------------------------------
// AppViewController
//-----------------------------------------------------------------------------------

@implementation AppViewController

-(instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    [[maybe_unused]] bool r = InitializeCommon();
    ASSERT_ALWAYS(r, "InitializeCommon() failed");
    
    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];
    
    if (!self.device)
    {
        NSLog(@"Metal is not supported");
        abort();
    }
    
    gMetalDevice = _device;

    ImGui::MyInitialize();

    // CGFloat dpiScale = NSScreen.mainScreen.backingScaleFactor;
    ImGui::LoadFonts(1.0f);
    
    // Setup Renderer backend
    ImGui_ImplMetal_Init(_device);

    return self;
}

-(MTKView *)mtkView
{
    return (MTKView *)self.view;
}

-(void)loadView
{
    AppSettings& s = GetSettings();
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0,
                                                          s.windowWidth == 0 ? 1200 : s.windowWidth,
                                                          s.windowHeight == 0 ? 720 : s.windowHeight)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];

    self.mtkView.device = self.device;
    self.mtkView.delegate = self;

    ImGui_ImplOSX_Init(self.view);
    
    [NSApp activateIgnoringOtherApps:YES];

    if (!Initialize()) {
        LOG_ERROR("%s initialization failed", CONFIG_APP_NAME);
        exit(-1);
    }
}

-(void)drawInMTKView:(MTKView*)view
{
    @autoreleasepool {        
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = view.bounds.size.width;
        io.DisplaySize.y = view.bounds.size.height;

        CGFloat framebufferScale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
        io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);

        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

        MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
        if (renderPassDescriptor == nil)
        {
            [commandBuffer commit];
            return;
        }

        // Start the Dear ImGui frame
        ImGui_ImplMetal_NewFrame(renderPassDescriptor);
        ImGui_ImplOSX_NewFrame(view);
        ImGui::NewFrame();
        ImGui::BeginFrame();

        // Our state (make them static = more or less global) as a convenience to keep the example terse.
        constexpr ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        Update();

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        renderPassDescriptor.colorAttachments[0].clearColor = 
            MTLClearColorMake(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
        ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
        [renderEncoder popDebugGroup];
        [renderEncoder endEncoding];

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present
        [commandBuffer presentDrawable:view.currentDrawable];
        [commandBuffer commit];

        MemTempAllocator::Reset();
    }
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    if (frameSize.width < 500)
        frameSize.width = 500;
    if (frameSize.height < 500)
        frameSize.height = 500;
    
    GetSettings().windowWidth = uint16(frameSize.width);
    GetSettings().windowHeight = uint16(frameSize.height);
    
    return frameSize;
}

//-----------------------------------------------------------------------------------
// Input processing
//-----------------------------------------------------------------------------------

- (void)viewWillAppear
{
    [super viewWillAppear];
    self.view.window.delegate = self;
}

- (void)windowWillClose:(NSNotification *)notification
{
    ImGui::SaveState();
    
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();

    ImGui::MyRelease();    
    Release();
    ReleaseCommon();

    ImGui::DestroyContext();
}
@end

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------
@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@end

@implementation AppDelegate

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

-(instancetype)init
{
    if (self = [super init])
    {
        NSViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self.window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                    backing:NSBackingStoreBuffered
                                                      defer:NO];
        self.window.contentViewController = rootViewController;
        [self.window center];
        [self.window makeKeyAndOrderFront:self];
    }
}

@end

void* CreateRGBATexture(uint32 width, uint32 height, const void* data)
{
    MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                            width:(NSUInteger)width
                                                                                            height:(NSUInteger)height
                                                                                            mipmapped:NO];
    textureDescriptor.usage = MTLTextureUsageShaderRead;
    textureDescriptor.storageMode = MTLStorageModeManaged;
    id <MTLTexture> texture = [gMetalDevice newTextureWithDescriptor:textureDescriptor];
    [texture replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)width, (NSUInteger)height) mipmapLevel:0 withBytes:data bytesPerRow:(NSUInteger)width * 4];
    return (__bridge void*)texture;
}

void DestroyTexture(void*)
{
}

// TODO:
bool SetClipboardString(const char* text)
{
    return false;
}

bool GetClipboardString(char* textOut, uint32 textSize)
{
    return false;
}


//-----------------------------------------------------------------------------------
// Application main() function
//-----------------------------------------------------------------------------------

int main(int argc, const char * argv[])
{
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        AppDelegate *delegate = [[AppDelegate alloc] init];
        app.delegate = delegate;

        [app run];
    }
}

#endif // PLATFORM_APPLE
