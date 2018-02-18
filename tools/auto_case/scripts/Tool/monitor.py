from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class FileMonitor(FileSystemEventHandler):
	def __init__(self):
		self._observer = Observer()

		self._event_callback = {
			'modified': [],
			'moved': [],
			'created': [],
			'deleted': [],
		}

	def start(self, directory, recursive=False):
		self._observer.schedule(self, directory, recursive)
		self._observer.start()

	def reset(self, directory, recursive=False):
		if self._observer.is_alive():
			self._observer.unschedule_all()
			self._observer.stop()
			self._observer.join()
		
		self._observer.schedule(self, directory, recursive)
		self._observer.start()

	def stop(self):
		self._observer.unschedule_all()
		self._observer.stop()
		self._observer.join()

	def bind(self, event, func):
		if event in self._event_callback:
			self._event_callback[event].append(func)
			return True

		return False
			
	def on_moved(self, event):
		"""Called when a file or a directory is moved or renamed.

		:param event:
			Event representing file/directory movement.
		:type event:
			:class:`DirMovedEvent` or :class:`FileMovedEvent`
		"""
		print( 'moved', event.src_path, event.dest_path )
		for cb in self._event_callback['moved']:
			cb(event)


	def on_created(self, event):
		"""Called when a file or directory is created.

		:param event:
			Event representing file/directory creation.
		:type event:
			:class:`DirCreatedEvent` or :class:`FileCreatedEvent`
		"""
		print( 'created', event.src_path )
		for cb in self._event_callback['created']:
			cb(event)


	def on_deleted(self, event):
		"""Called when a file or directory is deleted.

		:param event:
			Event representing file/directory deletion.
		:type event:
			:class:`DirDeletedEvent` or :class:`FileDeletedEvent`
		"""
		print( 'deleted', event.src_path )
		for cb in self._event_callback['deleted']:
			cb(event)


	def on_modified(self, event):
		"""Called when a file or directory is modified.

		:param event:
			Event representing file/directory modification.
		:type event:
			:class:`DirModifiedEvent` or :class:`FileModifiedEvent`
		"""
		print( 'modified', event.src_path )
		for cb in self._event_callback['modified']:
			cb(event)


if __name__ == '__main__':
	import os
	import time
	monitor = FileMonitor(None, os.getcwd())
	try:
		monitor.start()
		while True:
			time.sleep(1)
	except KeyboardInterrupt:
		monitor.stop()
	