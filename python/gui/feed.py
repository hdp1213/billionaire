from datetime import datetime
import enum
import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk, GObject


class FeedField(enum.IntEnum):
    TIMESTAMP = 0
    COMMAND = 1
    CONTENT = 2

    def __str__(self):
        return f'{self.name.replace("_", " ").title()}'


class FeedData(Gtk.ListStore):
    """Widget containing feed text"""
    def __init__(self):
        Gtk.ListStore.__init__(self, str, str, str)

        self._iters = {}
        self._content = {}
        self.last_msg = 0

    def append_message(self, date_time, command, content):
        timestamp = date_time.timestamp()
        time_str = FeedData.datetime_to_string(date_time)

        message = [time_str, f'{command}:', content]
        self._iters[timestamp] = self.append(message)
        self._content[timestamp] = (command, content)
        self.last_msg = len(self._content) - 1

    @classmethod
    def datetime_to_string(cls, date_time):
        second_str = float(f'{date_time:%S.%f}')
        return f'[{date_time:%H:%M:}{second_str:06.3f}]'

    def clear_all(self):
        self.clear()
        self._iters = {}
        self._content = {}
        self.last_msg = 0


class FeedTable(Gtk.ScrolledWindow):
    """The thing that shows a FeedData buffer for eyes"""
    def __init__(self, feed_data):
        Gtk.ScrolledWindow.__init__(self)

        self.view = Gtk.TreeView(feed_data)

        self.column_names = [str(field) for field in FeedField]
        self.columns = [None] * len(self.column_names)

        def init_column(col, field, cell_renderer=Gtk.CellRendererText):
            cell = cell_renderer()

            col.pack_start(cell, True)
            col.add_attribute(cell, 'text', field)

        for field in FeedField:
            self.columns[field] = Gtk.TreeViewColumn(self.column_names[field])
            init_column(self.columns[field], field)
            self.view.append_column(self.columns[field])

        self.view.set_headers_visible(False)

        self.set_min_content_width(420)
        self.set_min_content_height(100)

        self.set_propagate_natural_width(True)
        self.set_propagate_natural_height(True)

        self.add(self.view)


class FeedDisplay(Gtk.Frame):
    """Display widget for feed"""
    def __init__(self):
        Gtk.Frame.__init__(self, label='Feed')

        self.data = FeedData()
        self.table = FeedTable(self.data)

        self.grid = Gtk.Grid()

        self.grid.attach(self.table, 1, 1, 1, 1)

        self.grid.set_column_spacing(15)
        self.grid.set_row_spacing(5)

        self.grid.set_margin_start(5)
        self.grid.set_margin_end(5)
        self.grid.set_margin_top(5)
        self.grid.set_margin_bottom(5)

        self.set_margin_start(5)
        self.set_margin_end(5)
        self.set_margin_top(5)
        self.set_margin_bottom(5)

        self.add(self.grid)

        self.set_name('feed-display')

    def add_to_feed(self, command, content):
        self.data.append_message(datetime.now(), command, content)
        path = Gtk.TreePath.new_from_indices([self.data.last_msg])
        self.table.view.scroll_to_cell(path, None, True, 1.0, 0.0)
