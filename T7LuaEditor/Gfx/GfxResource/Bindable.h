//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_BINDABLE_H
#define T7LUAEDITOR_BINDABLE_H
#include "GraphicsResource.h"
#include <wrl/client.h>
namespace wrl = Microsoft::WRL;
class Bindable : public GraphicsResource {

public:
    virtual void Bind(Renderer& gfx) = 0;
//    virtual void InitializeParentReference( const Drawable& ) noexcept
//    {}
    virtual std::string GetUID() const noexcept
    {
        assert( false );
        return "";
    }
    virtual ~Bindable() = default;
};


#endif //T7LUAEDITOR_BINDABLE_H
