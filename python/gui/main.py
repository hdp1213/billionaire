import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Gdk', '3.0')

from gi.repository import Gdk, Gtk, GObject

from hand import HandDisplay
from offer import OfferDisplay


class ClientApplication(Gtk.Window):
    """Client frontend GUI for Billionaire playing"""
    def __init__(self):
        super(ClientApplication, self).__init__(default_height=50,
                                                default_width=200)
        self.connect('delete-event', Gtk.main_quit)

        self.set_title('Billionaire')

        self.grid = Gtk.Grid()

        self.hand = HandDisplay()
        self.grid.attach(self.hand, 1, 1, 1, 1)

        self.offer = OfferDisplay()
        self.grid.attach(self.offer, 1, 2, 1, 1)

        self.add(self.grid)

        css_provider = Gtk.CssProvider()
        # css_provider.load_from_path('style/main.css')

        context = Gtk.StyleContext()
        context.add_provider_for_screen(Gdk.Screen.get_default(),
                                        css_provider,
                                        Gtk.STYLE_PROVIDER_PRIORITY_USER)

        self.show_all()


if __name__ == '__main__':
    # Calling GObject.threads_init() is not needed for PyGObject 3.10.2+
    GObject.threads_init()

    ClientApplication()
    Gtk.main()
