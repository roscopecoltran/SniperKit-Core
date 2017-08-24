from peewee import CharField, DateTimeField, ForeignKeyField

from mine.models.base import Base
from mine.models.repo import Repo

class Commit(Base):
    committed_at = DateTimeField()
    committed_by = CharField()
    message = CharField(null=True)
    repo = ForeignKeyField(Repo, related_name="commits")
    sha = CharField(null=True)
