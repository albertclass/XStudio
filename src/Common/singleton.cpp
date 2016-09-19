#include "defines.h"
#include <windows.h>
#include "singleton.h"


namespace xgc
{
	namespace common
	{
		#ifdef ENABLE_NEW_SETLONGLIVITY
		Private::TrackerArray* Private::pTrackerArray = 0;
		#else
		Private::TrackerArray Private::pTrackerArray = 0;
		unsigned int Loki::Private::elements = 0;
		#endif

		////////////////////////////////////////////////////////////////////////////////
		// function AtExitFn
		// Ensures proper destruction of objects with longevity
		////////////////////////////////////////////////////////////////////////////////

		#ifdef ENABLE_NEW_SETLONGLIVITY

		void __cdecl Private::AtExitFn()
		{
			assert(pTrackerArray!=0 && !pTrackerArray->empty());
    
			// Pick the element at the top of the stack
			LifetimeTracker* pTop = pTrackerArray->back();
    
			// Remove that object off the stack _before_ deleting pTop
			pTrackerArray->pop_back();
    
			// Destroy the element
			delete pTop;
    
			// Destroy stack when it's empty _after_ deleting pTop
			if(pTrackerArray->empty())
			{
				delete pTrackerArray;
				pTrackerArray = 0;
			}
		}

		#else

		void __cdecl Private::AtExitFn()
		{
			assert(elements > 0 && pTrackerArray != 0);
			// Pick the element at the top of the stack
			LifetimeTracker* pTop = pTrackerArray[elements - 1];
			// Remove that object off the stack
			// Don't check errors - realloc with less memory 
			//     can't fail
			pTrackerArray = static_cast<TrackerArray>(std::realloc(
				pTrackerArray, sizeof(*pTrackerArray) * --elements));
			// Destroy the element
			delete pTop;
		}

		#endif 
	}
}