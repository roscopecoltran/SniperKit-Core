from peewee import Model

from mine.settings import DATABASE

class Base(Model):
    @classmethod
    def fields(cls):
        return [
            key for key in cls.__dict__.keys()
            if cls.__dict__[key].__class__.__name__ == "FieldDescriptor"
        ]

    @classmethod
    def as_dict(cls, model):
        return {field: getattr(model, field) for field in cls.fields()}

    class Meta:
        database = DATABASE
