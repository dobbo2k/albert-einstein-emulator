// Albert -- the emulated screen, as a panel and as the window.
//
// Split out of main.cpp at M10 phase 4.
#include "app.h"

#ifdef _WIN32
// After app.h, and with both guards: <windows.h> otherwise drags in winsock
// and a min/max macro pair that breaks std::min in this very file.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace albert {

// ------------------------------------------------------- Application icon

// Read the icon back out of the running executable's resources. The icon is
// already there for Explorer and the taskbar (src/app/albert.rc), so taking it
// from there rather than embedding the pixels a second time means the About
// window cannot drift away from the one on the taskbar -- and costs the binary
// nothing.
SDL_Texture* load_app_icon(SDL_Renderer* renderer, int px) {
#ifdef _WIN32
    // Resource 1, matching albert.rc. LoadImage picks the entry nearest the
    // requested size out of the group.
    HICON icon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1),
                                               IMAGE_ICON, px, px, LR_DEFAULTCOLOR));
    if (!icon) return nullptr;

    ICONINFO info{};
    if (!GetIconInfo(icon, &info)) {
        DestroyIcon(icon);
        return nullptr;
    }

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = px;
    bi.bmiHeader.biHeight = -px;  // negative for top-down, the way SDL wants it
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(px) * px, 0);
    HDC dc = GetDC(nullptr);
    const int rows = GetDIBits(dc, info.hbmColor, 0, px, pixels.data(), &bi, DIB_RGB_COLORS);
    ReleaseDC(nullptr, dc);

    // GetDIBits is documented to fill the fourth byte only for bitmaps that
    // carry one, and hands back zeroes otherwise -- which would make the whole
    // image transparent rather than merely un-antialiased. Fall back to the
    // icon's AND mask, which is where transparency lived before 32-bit icons.
    bool any_alpha = false;
    for (std::uint32_t p : pixels) {
        if ((p >> 24) != 0) { any_alpha = true; break; }
    }
    if (rows && !any_alpha) {
        std::vector<std::uint32_t> mask(static_cast<std::size_t>(px) * px, 0);
        dc = GetDC(nullptr);
        const int mrows = GetDIBits(dc, info.hbmMask, 0, px, mask.data(), &bi, DIB_RGB_COLORS);
        ReleaseDC(nullptr, dc);
        for (std::size_t i = 0; i < pixels.size(); ++i) {
            // The mask is 1 where the icon is see-through; it comes back here
            // expanded to white or black.
            const bool clear = mrows && (mask[i] & 0x00ffffffu) != 0;
            pixels[i] = clear ? 0u : (pixels[i] | 0xff000000u);
        }
    }

    DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
    DestroyIcon(icon);
    if (!rows) return nullptr;

    // 32-bit BI_RGB is 0xAARRGGBB in a DWORD, which is what ARGB8888 names.
    SDL_Surface* surf = SDL_CreateSurfaceFrom(px, px, SDL_PIXELFORMAT_ARGB8888, pixels.data(),
                                              px * static_cast<int>(sizeof(std::uint32_t)));
    if (!surf) return nullptr;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
#else
    (void)renderer;
    (void)px;
    return nullptr;
#endif
}


// ------------------------------------------------------------- Screen panel

// The texture is refreshed every frame in both modes: emulator mode blits it
// straight to the window, the Screen panel puts it in an ImGui window.
void update_screen_texture(AppState& app, SDL_Renderer* renderer) {
    if (app.screen_tex == nullptr) {
        app.screen_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           ein::Tms9129::kWidth, ein::Tms9129::kHeight);
        if (app.screen_tex) SDL_SetTextureScaleMode(app.screen_tex, SDL_SCALEMODE_NEAREST);
    }

    app.machine.vdp.render(app.framebuffer.data());
    if (app.screen_tex) {
        SDL_UpdateTexture(app.screen_tex, nullptr, app.framebuffer.data(),
                          ein::Tms9129::kWidth * static_cast<int>(sizeof(std::uint32_t)));
    }
}

// Where the picture goes in emulator mode: the viewport work area -- what is
// left of the window once the menu bar and the status bar have taken their
// strips -- with the picture centred in it. 256x192 is 4:3, the same shape as
// the television it was drawn for, so scaling it squarely is already
// aspect-correct; there is no pixel aspect to correct for on this machine.
SDL_FRect emulator_screen_rect(const AppState& app) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float aw = vp->WorkSize.x;
    const float ah = vp->WorkSize.y;
    float scale = std::min(aw / static_cast<float>(ein::Tms9129::kWidth),
                           ah / static_cast<float>(ein::Tms9129::kHeight));
    if (app.integer_scale) scale = std::max(1.0f, std::floor(scale));
    const float w = static_cast<float>(ein::Tms9129::kWidth) * scale;
    const float h = static_cast<float>(ein::Tms9129::kHeight) * scale;
    // WorkPos is viewport-absolute; the renderer wants window-relative.
    return SDL_FRect{vp->WorkPos.x - vp->Pos.x + (aw - w) * 0.5f,
                     vp->WorkPos.y - vp->Pos.y + (ah - h) * 0.5f, w, h};
}

void draw_screen_panel(AppState& app) {
    if (!app.show_screen) return;
    if (!ImGui::Begin("Screen", &app.show_screen)) { ImGui::End(); return; }

    app.screen_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    ImGui::Checkbox("fit", &app.screen_fit);
    if (!app.screen_fit) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderInt("zoom", &app.screen_zoom, 1, 4);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%d x %d", ein::Tms9129::kWidth, ein::Tms9129::kHeight);

    if (app.screen_tex) {
        float w = static_cast<float>(ein::Tms9129::kWidth * app.screen_zoom);
        float h = static_cast<float>(ein::Tms9129::kHeight * app.screen_zoom);
        if (app.screen_fit) {
            // Same rule as emulator mode: square pixels, centred, as large as
            // the space allows. Without this the image is pinned to a zoom
            // factor and simply gets clipped by a smaller panel.
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            const float scale = std::min(avail.x / ein::Tms9129::kWidth,
                                         avail.y / ein::Tms9129::kHeight);
            if (scale > 0.0f) {
                w = ein::Tms9129::kWidth * scale;
                h = ein::Tms9129::kHeight * scale;
                const float pad = (avail.x - w) * 0.5f;
                if (pad > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pad);
            }
        }
        // The SDL3 backend overwrites every texture's scale mode as it draws,
        // and its default is linear -- which turns 256x192 pixel art to mush
        // the moment it is magnified. Ask for point sampling around the image
        // and put it back afterwards, or the font atlas gets it too.
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest, nullptr);
        ImGui::Image(reinterpret_cast<ImTextureID>(app.screen_tex), ImVec2(w, h));
        dl->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerLinear, nullptr);
    }
    ImGui::End();
}

}  // namespace albert
