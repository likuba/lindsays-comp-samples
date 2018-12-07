#include "pch.h"
#include "MainPage.h"
#include <winrt/Windows.UI.XAML.Hosting.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.Interactions.h>


using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI;
using namespace Windows::UI::Composition::Interactions;

namespace winrt::InteractionTracker_WinRTcpp::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
		InitializeComposition();
    }

    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }

	void MainPage::InitializeComposition()
	{
		Visual visual = ElementCompositionPreview::GetElementVisual(MainGrid());
		Compositor compositor = visual.Compositor();

		SpriteVisual blueRect = compositor.CreateSpriteVisual();
		blueRect.Size({ 500.0f, 500.0f } );
		blueRect.Brush(compositor.CreateColorBrush(Colors::Blue()));
		ElementCompositionPreview::SetElementChildVisual(MainGrid(), blueRect);

		InteractionTracker tracker = InteractionTracker::Create(compositor);
		//root.Brush(g_compositor.CreateColorBrush({ 0xFF, 0xEF, 0xE4 , 0xB0 }));
		//g_target.Root(root);
		//root.Offset = (10.0f, 10.0f, 0.0f);
	}
}
