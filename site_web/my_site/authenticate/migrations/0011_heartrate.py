# Generated by Django 4.2 on 2023-04-28 23:58

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('authenticate', '0010_delete_heartrate'),
    ]

    operations = [
        migrations.CreateModel(
            name='HeartRate',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('timestamp', models.DateTimeField(auto_now_add=True)),
                ('bpm', models.IntegerField()),
                ('famille', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='famille', to='authenticate.famille')),
            ],
        ),
    ]