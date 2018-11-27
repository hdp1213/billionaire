import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Gdk', '3.0')

from gi.repository import Gdk, Gtk, GObject


class BillionaireMenu(Gtk.MenuBar):
    """The menu for this GUI"""
    def __init__(self, accel_group):
        Gtk.MenuBar.__init__(self)

        self.file = Gtk.MenuItem('File')

        file_menu = Gtk.Menu()

        file_menu.set_accel_group(accel_group)

        connect = Gtk.MenuItem('Connect')
        connect.add_accelerator('activate', accel_group, ord('N'),
                                Gdk.ModifierType.CONTROL_MASK,
                                Gtk.AccelFlags.VISIBLE)
        connect.connect('activate', self.on_connect_menu)

        quit = Gtk.MenuItem('Quit')
        quit.add_accelerator('activate', accel_group, ord('Q'),
                             Gdk.ModifierType.CONTROL_MASK,
                             Gtk.AccelFlags.VISIBLE)
        quit.connect('activate', Gtk.main_quit)

        file_menu.append(connect)
        file_menu.append(quit)

        self.file.set_submenu(file_menu)
        self.append(self.file)

    def on_connect_menu(self, widget):
        menu = ConnectionDialog()

        menu.show()


class ConnectionDialog(Gtk.Window):
    """Where the connection information goes :)"""
    @GObject.Signal
    def server_connect(self, ip: str):
        pass

    def __init__(self):
        Gtk.Window.__init__(self)
        self.set_title('Connect')

        self.grid = Gtk.Grid()

        self.name_label = Gtk.Label('Player Name')
        self.name = Gtk.Entry()
        self.name.set_max_length(32)

        self.grid.attach(self.name_label, 1, 1, 1, 1)
        self.grid.attach(self.name, 2, 1, 1, 1)

        self.ip_label = Gtk.Label('Server IP')
        self.ip = Gtk.Entry()
        self.ip.set_max_length(32)
        self.ip.set_placeholder_text('127.0.0.1')
        self.ip.set_input_purpose(Gtk.InputPurpose.NUMBER)
        self.ip.set_width_chars(16)

        self.grid.attach(self.ip_label, 1, 2, 1, 1)
        self.grid.attach(self.ip, 2, 2, 1, 1)

        self.hbox = self.make_buttons()
        self.grid.attach(self.hbox, 3, 3, 1, 1)

        self.grid.set_column_spacing(15)
        self.grid.set_row_spacing(5)

        self.grid.set_margin_start(5)
        self.grid.set_margin_end(5)
        self.grid.set_margin_top(5)
        self.grid.set_margin_bottom(5)

        # self.set_margin_start(5)
        # self.set_margin_end(5)
        # self.set_margin_top(5)
        # self.set_margin_bottom(5)

        self.add(self.grid)

        self.show_all()

    def make_buttons(self):
        connect_button = Gtk.Button('Connect')
        cancel_button = Gtk.Button('Cancel')

        connect_button.connect('clicked', self.on_connect)
        cancel_button.connect('clicked', self.on_cancel)

        hbox = Gtk.Box(Gtk.Orientation.HORIZONTAL, 5)
        hbox.pack_start(connect_button, expand=False, fill=False, padding=0)
        hbox.pack_start(cancel_button, expand=False, fill=False, padding=0)

        return hbox

    def on_connect(self, button):
        print('ah fuk')
        self.emit('server-connect', self.ip.get_text())
        self.close()

    def on_cancel(self, button):
        self.close()
