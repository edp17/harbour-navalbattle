import QtQuick 2.0
import Sailfish.Silica 1.0

import "pages" as Pages
import "cover" as Covers

ApplicationWindow
{
    initialPage: Component { Pages.GamePage { } }
    cover: Component { Covers.CoverPage { } }
}
