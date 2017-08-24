from peewee import CharField, ForeignKeyField, IntegerField

from mine.models.base import Base
from mine.models.repo import Repo

class Language(Base):
    repo = ForeignKeyField(Repo, related_name="languages")
    name = CharField()
    size_in_bytes = IntegerField(default=0)
