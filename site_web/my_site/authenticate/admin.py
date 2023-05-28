from django.contrib import admin

from .models import DerniereLigne
from .models import HeartRate
from .models import famille 
admin.site.register(famille)
admin.site.register(DerniereLigne)
admin.site.register(HeartRate)


